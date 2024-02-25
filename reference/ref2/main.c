#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>

#include "utilities/block.h"
#include "utilities/types.h"
#include "utilities/record.h"
#include "utilities/b_tree_index.h"
#include "utilities/linear_scan.h"

int main(void) {
    // Simulate disk using an array storing pointers to each block
    byte_t *disk[num_of_blocks];
    //byte_t *disk2[num_of_blocks];

    printf("Initializing disk of size %ld B...\n", DISK_CAPACITY);
    for (int i = 0; i < num_of_blocks; i++) {
        byte_t *blockPtr;
        blockPtr = (byte_t*)calloc(BLOCK_SIZE, sizeof(byte_t));

        if (blockPtr == NULL) {
            printf("Failed to allocate block\n");

            for (int j = 0; j < i; j++) {
                free(disk[j]);
            };
            
            return -1;
        }

        disk[i] = blockPtr; 
    }

    // printf("Initializing disk2 of size %ld B...\n", DISK_CAPACITY);
    // for (int i = 0; i < num_of_blocks; i++) {
    //     byte_t *blockPtr;
    //     blockPtr = (byte_t*)calloc(BLOCK_SIZE, sizeof(byte_t));

    //     if (blockPtr == NULL) {
    //         printf("Failed to allocate block\n");

    //         for (int j = 0; j < i; j++) {
    //             free(disk[j]);
    //         };
            
    //         return -1;
    //     }

    //     disk2[i] = blockPtr; 
    // }

    printf("Initializing blocks...\n");

    //FILE* fp = fopen("./data/games_mini_duplicates_non_sequential.txt", "r");
    //FILE* fp = fopen("./data/games_mini_duplicates.txt", "r");
    //FILE* fp = fopen("./data/games_mini.txt", "r");
    FILE* fp = fopen("./data/games.txt", "r");
    char buffer [110];
    int recordCtr, blockCtr = 0;
    int blocksInUse = 0;
    if (!fp) {
        perror("Failed to open file");
    } else {
        printf("Loading data to disk...\n");
        // Discard header
        fgets(buffer, 110, fp);

        int len = strlen(buffer);
        if (buffer[len - 1] == '\n') {
            buffer[len - 1] = 0;
        };

        while (!feof(fp)) {
            if (buffer != fgets(buffer, 100, fp)) {
                break;
            }

            int len = strlen(buffer);
            if (buffer[len - 1] == '\n') {
                buffer[len - 1] = 0;
            };
            
            uint8_t day;
            uint8_t month;
            uint16_t year;
            uint32_t home_team_id;
            uint8_t pts_home;
            double_t fg_pct_home;
            double_t ft_pct_home;
            double_t fg3_pct_home;
            uint8_t ast_home;
            uint8_t reb_home;
            uint8_t home_team_wins;

            int ret = sscanf(buffer, "%"SCNu8"/%"SCNu8"/%"SCNu16"\t%"SCNu32"\t%"SCNu8"\t%lf\t%lf\t%lf""\t%"SCNu8"\t%"SCNu8"\t%"SCNu8, 
            &day, &month, &year, &home_team_id, &pts_home, &fg_pct_home, &ft_pct_home, &fg3_pct_home, &ast_home, &reb_home, &home_team_wins);
           
            record rec;
            pack_record(&rec, day, month, year, home_team_id, pts_home, fg_pct_home, ft_pct_home, fg3_pct_home, ast_home, reb_home, home_team_wins);

            for (int i = 0; i < num_of_blocks; i++) {
                byte_t *blockPtr = disk[i];
                //byte_t *blockPtr2 = disk2[i];

                if (write_to_block(blockPtr, rec.data, sizeof(record), 0) == NULL) {

                    if (i == num_of_blocks - 1) {
                        printf("Disk full. Aborting.\n");

                        for (int k = 0; k < num_of_blocks; k++) {
                            free(disk[k]);
                        };
                        return -1;
                    }
                } else {
                    recordCtr++;
                    blocksInUse = i + 1;
                    break;
                }

                // if (write_to_block(blockPtr2, rec.data, sizeof(record), 0) == NULL) {

                //     if (i == num_of_blocks - 1) {
                //         printf("Disk 2 full. Aborting.\n");

                //         for (int k = 0; k < num_of_blocks; k++) {
                //             free(disk2[k]);
                //         };
                //         return -1;
                //     }
                // } else {
                //     break;
                // }
            };
        }
    };

    // Create BTREE
    printf("Creating B+ Tree Index...\n");
    Node* root = createNode(1); //Root Node is leaf at start
    setGlobal(root); //set root as root of tree
    for (int i = 0; i < blocksInUse; i++) {
        byte_t *blockPtr = disk[i];
        for (int j = 2; j < BLOCK_SIZE; j += sizeof(record) + 1) {
            uint8_t r_day;
            uint8_t r_month;
            uint16_t r_year;
            uint32_t r_home_team_id;
            uint8_t r_pts_home;
            double_t r_fg_pct_home;
            double_t r_ft_pct_home;
            double_t r_fg3_pct_home;
            uint8_t r_ast_home;
            uint8_t r_reb_home;
            uint8_t r_home_team_wins;

            record r_rec;
            if ((byte_t *) &(r_rec.data) == read_from_block(blockPtr, j, r_rec.data, 0)) {
                unpack_record(
                    &r_rec, &r_day, &r_month, &r_year, &r_home_team_id, 
                    &r_pts_home, &r_fg_pct_home, &r_ft_pct_home, &r_fg3_pct_home, 
                    &r_ast_home, &r_reb_home, &r_home_team_wins);

                RecordPtr* p = malloc(sizeof(RecordPtr));
                p->blockPtr=blockPtr;
                p->offset=j;
                Element* e = malloc(sizeof(Element));
                Node* newNode = insert(root,r_fg_pct_home,p,e);
                //If a new parent Node needs to be added
                if(e->ptr!=NULL){
                    Node* superNode = createNode(0); //Internal Node
                    superNode->children[0] = root;
                    superNode->children[1] = newNode;
                    superNode->numKeys++;
                    (&superNode->keys[0])->value = e->key;
                    if (e->dupli) {
                        (&superNode->keys[0])->type = REPEATED;
                    } else {
                        (&superNode->keys[0])->type = FIRST_OCCURENCE;
                    }
                    root = superNode;
                    setGlobal(root);
                }
            } else {
                //printf("Failed to retrieve data\n");
            }
        }
    }
    printf("B+ Tree Index successfully created\n");
    printf("Number of Nodes: %d\n", countNumNodes(root));
    printf("Number of levels: %d\n", countNumLevels(root));
    printf("Keys in the root node: ");

    for (int i = 0; i < root->numKeys; i++) {
        printf("%f ", (&root->keys[i])->value);
    }
    printf("\n\n");

    // Benchmarking tools
    clock_t start_time_retrieval;
    clock_t end_time_retrieval;
    double_t cpu_time_retrieval;

    printf("\nEXPERIMENT 3 : Retrieve those records with the 'FG_PCT_home' equal to 0.5 (B+ Tree Index)\n");
    /**
     * EXPERIMENT 3: : Retrieve those records with the "FG_PCT_home" equal to 0.5
    */
    // Start benchmarking
    start_time_retrieval = clock();
    double_t fg_pct_home_target = 0.5;
    int internalNodesCtr = 0; // Number of internal nodes accessed
    printf("Retrieving records with FG_PCT_HOME=%.3lf\n", fg_pct_home_target);
    Node *startingLeafNode = search(root, fg_pct_home_target, &internalNodesCtr);

    if (!startingLeafNode) {
        printf("No records found with FG_PCT_HOME=%.3lf\n", fg_pct_home_target);
    } else {
        // Traverse leaf nodes
        Node *cur = startingLeafNode;
        int continueChecking = 1; // Continue checking if last key in the cur leaf node is in the search range
        double_t fg3_pct_home_total = 0.0;
        int recCtr = 0; // Number of records retrieved
        int leafNodesCtr = 0; // Number of leaf nodes accessed
        int blockCtr = 0;
        while (continueChecking) {
            leafNodesCtr++;
            for (int i = 0; i < cur->numKeys; i++) {
                if ((&cur->keys[i])->value == fg_pct_home_target) {
                    recCtr++;

                    uint8_t r_day;
                    uint8_t r_month;
                    uint16_t r_year;
                    uint32_t r_home_team_id;
                    uint8_t r_pts_home;
                    double_t r_fg_pct_home;
                    double_t r_ft_pct_home;
                    double_t r_fg3_pct_home;
                    uint8_t r_ast_home;
                    uint8_t r_reb_home;
                    uint8_t r_home_team_wins;

                    record r_rec;
                    RecordPtr* recPtr = (RecordPtr*) cur->children[i];
                    byte_t* blockPtr = recPtr->blockPtr;
                    int16_t offset = recPtr->offset;
                    if ((byte_t *) &(r_rec.data) == read_from_block(blockPtr, offset, r_rec.data,0)) {
                        unpack_record(
                            &r_rec, &r_day, &r_month, &r_year, &r_home_team_id, 
                            &r_pts_home, &r_fg_pct_home, &r_ft_pct_home, &r_fg3_pct_home, 
                            &r_ast_home, &r_reb_home, &r_home_team_wins);
                        fg3_pct_home_total += r_fg3_pct_home;
                        blockCtr++;
                    } else {
                        //printf("Failed to retrieve data\n");
                    }
                }

                // Check last key in cur leaf node
                if ((i == cur->numKeys - 1) && (&cur->keys[i])->value != fg_pct_home_target) {
                    continueChecking = 0;
                }
            }

            if (cur->children[MAX_KEYS] == NULL) {
                continueChecking = 0;
            } else {
                cur = cur->children[MAX_KEYS];
            }
        }
        // Stop benchmarking
        end_time_retrieval = clock();
        cpu_time_retrieval = (double_t)(end_time_retrieval - start_time_retrieval) / CLOCKS_PER_SEC;
        double_t fg3_pct_home_average = fg3_pct_home_total / recCtr;
        printf("Retrieval complete\n");
        printf("Internal Nodes Accessed: %d\n", internalNodesCtr);
        printf("Leaf Nodes Accessed: %d\n", leafNodesCtr);
        printf("Total Index Nodes Accessed: %d\n", internalNodesCtr + leafNodesCtr);
        printf("Total Data Blocks Accessed: %d\n", blockCtr);
        printf("Total Records Accessed: %d\n", recCtr);
        printf("Average FG3_PCT_HOME: %.3lf\n", fg3_pct_home_average);
        printf("Time taken for retrieval: %lf\n", cpu_time_retrieval);
    }

    printf("\nEXPERIMENT 3 : Retrieve those records with the 'FG_PCT_home' equal to 0.5 (Linear Scan)\n");
    // Start benchmarking
    start_time_retrieval = clock();
    double_t fg3_pct_home_avg = linearScanPoint(disk, blocksInUse, fg_pct_home_target);
    // Stop benchmarking
    end_time_retrieval = clock();
    cpu_time_retrieval = (double_t)(end_time_retrieval - start_time_retrieval) / CLOCKS_PER_SEC;
    printf("Time taken for retrieval: %lf\n", cpu_time_retrieval);

    printf("\nEXPERIMENT 4: Retrieve those records with the attribute 'FG_PCT_home' from 0.6 to 1, both inclusively (B+ Tree Index)\n");
    /**
     * EXPERIMENT 4: Retrieve those records with the attribute "FG_PCT_home" from 0.6 to 1, both inclusively
    */
    // Start benchmarking
    start_time_retrieval = clock();
    double_t fg_pct_home_target_lower = 0.6;
    double_t fg_pct_home_target_upper = 1.0;
    internalNodesCtr = 0; // Number of internal nodes accessed
    printf("Retrieving records with %.3lf <= FG_PCT_HOME <= %.3lf\n", fg_pct_home_target_lower, fg_pct_home_target_upper);
    startingLeafNode = search(root, fg_pct_home_target_lower, &internalNodesCtr);

    if (!startingLeafNode) {
        printf("No records found with FG_PCT_HOME=%.3lf\n", fg_pct_home_target);
    } else {
        // Traverse leaf nodes
        Node *cur = startingLeafNode;
        int continueChecking = 1; // Continue checking if last key in the cur leaf node is in the search range
        double_t fg3_pct_home_total = 0.0;
        int recCtr = 0; // Number of records retrieved
        int leafNodesCtr = 0; // Number of leaf nodes accessed
        int blockCtr = 0;
        while (continueChecking) {
            leafNodesCtr++;
            for (int i = 0; i < cur->numKeys; i++) {
                if ((&cur->keys[i])->value >= fg_pct_home_target_lower && (&cur->keys[i])->value <= fg_pct_home_target_upper) {
                    recCtr++;

                    uint8_t r_day;
                    uint8_t r_month;
                    uint16_t r_year;
                    uint32_t r_home_team_id;
                    uint8_t r_pts_home;
                    double_t r_fg_pct_home;
                    double_t r_ft_pct_home;
                    double_t r_fg3_pct_home;
                    uint8_t r_ast_home;
                    uint8_t r_reb_home;
                    uint8_t r_home_team_wins;

                    record r_rec;
                    RecordPtr* recPtr = (RecordPtr*) cur->children[i];
                    byte_t* blockPtr = recPtr->blockPtr;
                    int16_t offset = recPtr->offset;
                    if ((byte_t *) &(r_rec.data) == read_from_block(blockPtr, offset, r_rec.data, 0)) {
                        unpack_record(
                            &r_rec, &r_day, &r_month, &r_year, &r_home_team_id, 
                            &r_pts_home, &r_fg_pct_home, &r_ft_pct_home, &r_fg3_pct_home, 
                            &r_ast_home, &r_reb_home, &r_home_team_wins);
                        fg3_pct_home_total += r_fg3_pct_home;
                        blockCtr++;
                    } else {
                        printf("Failed to retrieve data\n");
                    }
                }

                // Check last key in cur leaf node
                if ((i == cur->numKeys - 1) && ((&cur->keys[i])->value > fg_pct_home_target_upper || (&cur->keys[i])->value < fg_pct_home_target_lower)) {
                    continueChecking = 0;
                }
            }

            if (cur->children[MAX_KEYS] == NULL) {
                continueChecking = 0;
            } else {
                cur = cur->children[MAX_KEYS];
            }
        }
        double_t fg3_pct_home_average = fg3_pct_home_total / recCtr;
        printf("Retrieval complete\n");
        printf("Internal Nodes Accessed: %d\n", internalNodesCtr);
        printf("Leaf Nodes Accessed: %d\n", leafNodesCtr);
        printf("Total Index Nodes Accessed: %d\n", internalNodesCtr + leafNodesCtr);
        printf("Total Data Blocks Accessed: %d\n", blockCtr);
        printf("Average FG3_PCT_HOME: %.3lf\n", fg3_pct_home_average);
        end_time_retrieval = clock();
        cpu_time_retrieval = (double_t)(end_time_retrieval - start_time_retrieval) / CLOCKS_PER_SEC;
        printf("Time taken for retrieval: %lf\n", cpu_time_retrieval);
    }

    printf("\nEXPERIMENT 4: Retrieve those records with the attribute 'FG_PCT_home' from 0.6 to 1, both inclusively (B+ Tree Index)\n");
    // Start benchmarking
    start_time_retrieval = clock();
    fg3_pct_home_avg = linearScanRange(disk, blocksInUse, fg_pct_home_target_lower, fg_pct_home_target_upper);
    // Stop benchmarking
    end_time_retrieval = clock();
    cpu_time_retrieval = (double_t)(end_time_retrieval - start_time_retrieval) / CLOCKS_PER_SEC;
    printf("Time taken for retrieval: %lf\n", cpu_time_retrieval);

    fg_pct_home_target = 0.35;
    /**
     * The code below is used for deletion. Due to disk limitations, either only the B+ Tree Index version or the linear scan version should be run. Comment out accordingly.
    */
    printf("\nEXPERIMENT 5: Delete those movies with the attribute 'FG_PCT_home' below 0.35 inclusively, update the B+ tree accordingly (B+ Tree Index)\n");
    // Start benchmarking
    start_time_retrieval = clock();
    deleteQuery(fg_pct_home_target, root, &root);
    fixLeaves(root);
    // Stop benchmarking
    end_time_retrieval = clock();
    cpu_time_retrieval = (double_t)(end_time_retrieval - start_time_retrieval) / CLOCKS_PER_SEC;
    printf("Time taken for retrieval: %lf\n", cpu_time_retrieval);

    printf("Number of Nodes: %d\n", countNumNodes(root));
    printf("Number of levels: %d\n", countNumLevels(root));
    printf("Keys in the root node: ");

    for (int i = 0; i < root->numKeys; i++) {
        printf("%f ", (&root->keys[i])->value);
    }
    printf("\n\n");

    // printf("\nEXPERIMENT 5: Delete those movies with the attribute 'FG_PCT_home' below 0.35 inclusively, update the B+ tree accordingly (Linear Scan)\n");
    // // Start benchmarking
    // start_time_retrieval = clock();
    // linearScanDelete(disk, blocksInUse, fg_pct_home_target);
    // // Stop benchmarking
    // end_time_retrieval = clock();
    // cpu_time_retrieval = (double_t)(end_time_retrieval - start_time_retrieval) / CLOCKS_PER_SEC;
    // printf("Time taken for retrieval: %lf\n", cpu_time_retrieval);

    for (int i = 0; i < num_of_blocks; i++) {
        free(disk[i]);
    };
    free(root);

    return 0;
}
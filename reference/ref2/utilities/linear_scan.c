#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "block.h"
#include "types.h"
#include "record.h"

double_t linearScanPoint(byte_t *disk[], int blocksInUse, double_t target) {
    double_t fg3_pct_home_total = 0;
    int blocks_found = 0;

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

                if (r_fg_pct_home == target) {
                    // printf("%"PRIu8"/%"PRIu8"/%"PRIu16"\t%"PRIu32"\t%"PRIu8"\t%.3lf\t%.3lf\t%.3lf\t%"PRIu8"\t%"PRIu8"\t%"PRIu8"\n", 
                    // r_day, r_month, r_year, r_home_team_id, r_pts_home, r_fg_pct_home, r_ft_pct_home, r_fg3_pct_home, r_ast_home, r_reb_home, r_home_team_wins);
                    fg3_pct_home_total += r_fg3_pct_home;
                    blocks_found++;
                }
            }
            else {
                //printf("Failed to retrieve data\n");
            }
        }
    }

    double_t avg = fg3_pct_home_total / blocks_found;
    printf("Average FG3_PCT_HOME: %.6lf\n", avg);

    return avg;
}


double_t linearScanRange(byte_t *disk[], int blocksInUse, double_t target_lower, double_t target_upper) {
    double_t fg3_pct_home_total = 0;
    int blocks_found = 0;

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

                if (r_fg_pct_home >= target_lower && r_fg_pct_home <= target_upper) {
                    // printf("%"PRIu8"/%"PRIu8"/%"PRIu16"\t%"PRIu32"\t%"PRIu8"\t%.3lf\t%.3lf\t%.3lf\t%"PRIu8"\t%"PRIu8"\t%"PRIu8"\n", 
                    // r_day, r_month, r_year, r_home_team_id, r_pts_home, r_fg_pct_home, r_ft_pct_home, r_fg3_pct_home, r_ast_home, r_reb_home, r_home_team_wins);
                    fg3_pct_home_total += r_fg3_pct_home;
                    blocks_found++;
                }
            }
            else {
                //printf("Failed to retrieve data\n");
            }
        }
    }

    double_t avg = fg3_pct_home_total / blocks_found;
    printf("Average FG3_PCT_HOME: %.6lf\n", avg);

    return avg;
}


void linearScanDelete(byte_t *disk[], int blocksInUse, double_t target) {
    // Deletes records with fg_pct_home <= target
    int blocks_found = 0;

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

                if (r_fg_pct_home <= target) {
                    // printf("%"PRIu8"/%"PRIu8"/%"PRIu16"\t%"PRIu32"\t%"PRIu8"\t%.3lf\t%.3lf\t%.3lf\t%"PRIu8"\t%"PRIu8"\t%"PRIu8"\n", 
                    // r_day, r_month, r_year, r_home_team_id, r_pts_home, r_fg_pct_home, r_ft_pct_home, r_fg3_pct_home, r_ast_home, r_reb_home, r_home_team_wins);
                    delete_from_block(blockPtr, j, 0);
                    blocks_found++;
                }
            }
            else {
                //printf("Failed to retrieve data\n");
            }
        }
    }

    printf("Deleted %"PRIu8" records\n", blocks_found);
}
# B+ Tree

This is a B+ tree implementation using C++ for NTU's CZ4031 course of Database Systems Principles. The data given is a list of movies, with each record containing the movie ID, its average rating and the number of votes.

|  tconst   | averageRating | numVotes |
| :-------: | :-----------: | :------: |
| tt0000001 |      5.6      |   1645   |
| tt0000002 |      6.1      |   198    |
| tt0000003 |      6.5      |   1342   |

There are more than 1 million records in the data. Our goal is to build a B+ tree index on the average rating field, and query the data based on the index for efficient retrieval.

## Implementation details:

- Blocks are stored in a memory pool of up to 500MB in main memory.
- Each block's size is 100B for the first implementation, and 500B for the second.
- Each record (movie) has a fixed size of ~20B.
- Multiple records can be stored per block.
- B+ tree's memory is dynamically allocated on creation.
- Each B+ tree node is the size of one block.
- Leaf nodes are linked in a doubly linked list.
- Leaf nodes maintain pointers to the actual data address in memory pool.

## Setup

- Ensure that you have a C++ compiler (we suggest [mingw](https://sourceforge.net/projects/mingw-w64/) for Windows).
- Setup your environment and ensure all C++ files are included in compilation.

  For example, if we use the [code runner](https://marketplace.visualstudio.com/items?itemName=formulahendry.code-runner) extension for VSCode, we would add this in `settings.json`:

  ```
  "code-runner.executorMap": {
    "cpp": "cd $dir && g++ *.cpp -o $fileNameWithoutExt && $dir$fileNameWithoutExt",
  }
  ```

  If we were running C++ using VSCode directly, we would define the `tasks.json` file with corresponding `args` to include all C++ files:

  ```
  "tasks": [
    {
      "args": ["-g", "${workspaceFolder}\\*.cpp", "-o", "${fileDirname}\\${fileBasenameNoExtension}.exe"],
    }
  ]
  ```

- `cd` to `main.cpp` under the `src` folder and compile the executable.

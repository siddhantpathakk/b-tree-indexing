/**
 * Defines the implementation for packing and unpacking records.
 * Note that the implementation is specific to the NBA dataset provided.
 * 
 * The records are also designed to be packed in a way to not span multiple blocks.
*/

#include <stdint.h>
#include <math.h>
#include "types.h"

void unpack_record(
    record *record, 
    uint8_t *day, uint8_t *month, uint16_t *year, 
    uint32_t *team_id_home,
    uint8_t *pts_home,
    double_t *fg_pct_home,
    double_t *ft_pct_home,
    double_t *fg3_pct_home,
    uint8_t *ast_home,
    uint8_t *reb_home,
    uint8_t *home_team_wins
) {
    // Restore Percentages
    uint16_t pct_tmp1, pct_tmp2;
    pct_tmp1 = record->data[0] & 0b111111; // Get last 6 bits
    pct_tmp2 = record->data[1] >> 4; // Get 1st 4 bits
    *fg_pct_home = ((pct_tmp1 << 4) | pct_tmp2) / (double) PERCENTAGE_DECIMAL_PLACES;

    pct_tmp1 = record->data[1] & 0b1111; // Get last 4 bits
    pct_tmp2 = record->data[2] >> 2; // Get 1st 6 bits
    *ft_pct_home = ((pct_tmp1 << 6) | pct_tmp2) / (double) PERCENTAGE_DECIMAL_PLACES;

    pct_tmp1 = record->data[2] & 0b11; // Get last 2 bits
    pct_tmp2 = record->data[3];
    *fg3_pct_home = ((pct_tmp1 << 8) | pct_tmp2) / (double) PERCENTAGE_DECIMAL_PLACES;

    // Restore Date
    *day = record->data[4] >> 3;

    uint8_t date_tmp1 = (record->data[4] & 0b111);
    *month = (date_tmp1 << 1) | (record->data[5] >> 7);

    uint16_t date_tmp2 = record->data[5] << 9;
    *year = (date_tmp2 >> 1) | record->data[6];

    // Restore home_team_wins and team_id_home
    *home_team_wins = record->data[7] >> 7;
    uint32_t id_tmp1, id_tmp2, id_tmp3, id_tmp4;
    id_tmp1 = record->data[7] & 0b01111111;
    id_tmp2 = record->data[8];
    id_tmp3 = record->data[9];
    id_tmp4 = record->data[10];
    *team_id_home = (id_tmp1 << 24) | (id_tmp2 << 16) | (id_tmp3 << 8) | id_tmp4; 

    *pts_home = record->data[11];
    *ast_home = record->data[12];
    *reb_home = record->data[13];
};


/**
 * In order to pack records compactly, we make use of the following observations:
 *      1. fg_pct_home, ft_pct_home and fg3_pct_home are percentage values stored as floats ranging from 0 to 1,
 *          with 3 decimal points of precision
 *          - Each can fit in 10 bits if we only store the fractional part of the value
 *      2. The max value for team_id_home can fit in 31 bits
 *      3. The max values for pts_home, ast_home and reb_home can each fit in 8 bits
 *      4. home_team_wins is a boolean value that can be represented with 1 bit
 * 
 * Then, we pack the records into (14 bytes) as follows:
 *      [Padding (2 bits)]
 *      [fg_pct_home(10 bits)]
 *      [ft_pct_home(10 bits)]
 *      [fg3_pct_home(10 bits)]
 *      [day(5 bits)]
 *      [month(4 bits)]
 *      [year(15 bits)]
 *      [home_team_wins(1 bit)]
 *      [team_id_home(31 bits)]
 *      [pts_home(8 bits)]
 *      [ast_home(8 bits)]
 *      [reb_home(8 bits)]
 * 2 bits of padding are left at the start of the record as the smallest alignment in C is a byte
*/ 
void pack_record(
    record *record, 
    uint8_t day, uint8_t month, uint16_t year, 
    uint32_t team_id_home,
    uint8_t pts_home,
    double fg_pct_home,
    double ft_pct_home,
    double fg3_pct_home,
    uint8_t ast_home,
    uint8_t reb_home,
    uint8_t home_team_wins
) { 
    // Store fg_pct_home in the 1st 4 bytes
    // Extract fractional parts
    double i_tmp;
    uint16_t fg_pct_home_f, ft_pct_home_f, fg3_pct_home_f;
    fg_pct_home_f = (uint16_t) (modf(fg_pct_home, &i_tmp) * PERCENTAGE_DECIMAL_PLACES);
    if (i_tmp == 1) fg_pct_home_f = (uint16_t) 1000;
    ft_pct_home_f = (uint16_t) (modf(ft_pct_home, &i_tmp) * PERCENTAGE_DECIMAL_PLACES);
    if (i_tmp == 1) ft_pct_home_f = (uint16_t) 1000;
    fg3_pct_home_f = (uint16_t) (modf(fg3_pct_home, &i_tmp) * PERCENTAGE_DECIMAL_PLACES);
    if (i_tmp == 1) fg3_pct_home_f = (uint16_t) 1000;

    record->data[0] = (fg_pct_home_f & 0b1111110000) >> 4; // Store 1st 6 bits of fg_pct_home_f
    record->data[1] = ((fg_pct_home_f & 0b1111) << 4) | (ft_pct_home_f >> 6); // Store last 4 bits of fg_pct_home_f and 1st 4 bits of ft_pct_home_f
    record->data[2] = ((ft_pct_home_f & 0b111111) << 2) | (fg3_pct_home_f >> 8); // Store last 6 bits of ft_pct_home_f and 1st 2 bits of fg3_pct_home_f
    record->data[3] = (fg3_pct_home_f & 0b11111111); // Store last 8 bits of fg3_pct_home_f

    // Store Date in the next 3 bytes
    record->data[4] = (day << 3) | (month >> 1);
    record->data[5] = ((month & 0b1) << 7) | (year >> 8);
    record->data[6] = (year & 0b11111111);

    // Store team_id_home in the next 4 bytes
    record->data[7] = (home_team_wins << 7) | (team_id_home >> 24);
    record->data[8] = (team_id_home & 0b111111110000000000000000) >> 16;
    record->data[9] = (team_id_home & 0b1111111100000000) >> 8;
    record->data[10] = (team_id_home & 0b11111111);

    // Store pts_home in the next byte
    record->data[11] = pts_home;

    // Store ast_home in the next byte
    record->data[12] = ast_home;

    // Store reb_home in the last byte
    record->data[13] = reb_home;
};

/**
 * Defines record I/O functions
*/

#ifndef RECORD_H
#define RECORD_H

#include <stdint.h>
#include <math.h>
#include "types.h"

/** @brief Reads a record in its compact form and returns the underlying values
 * 
 *  @param record The address of the record
 *  @param day The address to which the day value will be stored
 *  @param month The address to which the month value will be stored
 *  @param year The address to which the year value will be stored
 *  @param team_id_home The address to which the team_id_home value will be stored
 *  @param pts_home The address to which the pts_home value will be stored
 *  @param fg_pct_home The address to which the fg_pct_home value will be stored
 *  @param ft_pct_home The address to which the ft_pct_home value will be stored
 *  @param fg3_pct_home The address to which the fg3_pct_home value will be stored
 *  @param ast_home The address to which the ast_home value will be stored
 *  @param reb_home The address to which the reb_home value will be stored
 *  @param home_team_wins The address to which the home_team_wins value will be stored
 * 
 *  @return Void 
 */
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
);

/** @brief Compacts the provided values into a record
 * 
 *  @param record The address of the record
 *  @param day The day value to be stored
 *  @param month The month value to be stored
 *  @param year The year value to be stored
 *  @param team_id_home The team_id_home value to be stored
 *  @param pts_home The pts_home value to be stored
 *  @param fg_pct_home The fg_pct_home value to be stored
 *  @param ft_pct_home The ft_pct_home value to be stored
 *  @param fg3_pct_home The fg3_pct_home value to be stored
 *  @param ast_home The ast_home value to be stored
 *  @param reb_home The reb_home value to be stored
 *  @param home_team_wins The home_team_wins value to be stored
 * 
 *  @return Void 
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
); 

#endif
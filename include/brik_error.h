/*
 * brik_error.h
 *
 *  Created on: Apr 23, 2019
 *      Author: BartKim
 */
#ifndef INCLUDE_BRIK_ERROR_H_
#define INCLUDE_BRIK_ERROR_H_

#define BRIK_STATUS_BASE      0

/** @enum SN_STATUS */
typedef enum brik_status {
    BRIK_STATUS_OK                           = (BRIK_STATUS_BASE + 0),  /**< 0 */
    BRIK_STATUS_TIMEOUT,                                                /**< 1 */
    BRIK_STATUS_INVALID_PARAM,                                          /**< 2 */
    BRIK_STATUS_NOT_SUPPORTED,                                          /**< 3 */
    BRIK_STATUS_UNKNOWN_MESSAGE,                                        /**< 4 */
    BRIK_STATUS_OUT_OF_MEM,                                             /**< 5 */
    BRIK_STATUS_NOT_INITIALIZED,                                        /**< 6 */
    BRIK_STATUS_ALREADY_INITIALIZED,                                    /**< 7 */
    BRIK_STATUS_RESOURCE_NOT_AVAILABLE,                                 /**< 8 */
    BRIK_STATUS_SDL_ERROR,                                              /**< 9 */
    BRIK_STATUS_DECODE_ERROR,                                           /**< 10 */
    BRIK_STATUS_NOT_OK                                                 /**< 11 */
} BRIK_STATUS;                                                          /**< SN3D Error Code */

/** @typedef ERROR_T */
typedef int        ERROR_T;

typedef enum error_status {
    ERROR_OK                                 = 0,
    ERROR_NOT_OK                             = (-1)
} ERROR_STATUS;

/*************************************************************
 * @name System Error Handler
 *  Description of Message Queue Init and Uninit funtions.
 *////@{

/** @brief
 *
 *  @param errorStatus
 *  @param errorMeesage
 *  @param _file
 *  @param _func
 *  @param _line
 *
 *  @return BRIK_STATUS
 *  @note
 */
extern void ERROR_StatusCheck_Inline(BRIK_STATUS errorStatus, const char* errorMessage, const char* _file, const char* _func, const int _line);


/** @brief
 *
 *  @param log
 *
 *  @return SN_STATUS
 *  @note
 */
extern void ERROR_SystemLog(const char* log);

/** @brief
 *
 *  @param errorStatus
 *  @param errorMessage
 *
 *  @return void
 *  @note
 */
#define ERROR_StatusCheck(errorStatus, errorMessage) \
{ERROR_StatusCheck_Inline(errorStatus, errorMessage, __FILE__, __FUNCTION__, __LINE__);}


/*************************************************************@}*/

#endif /* INCLUDE_BRIK_ERROR_H_ */

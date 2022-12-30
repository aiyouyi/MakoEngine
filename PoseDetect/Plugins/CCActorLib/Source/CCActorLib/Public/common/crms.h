#ifndef CRMS_H
#define CRMS_H

#include "core/inc.h"

/* audio RMS param module */
typedef struct
{
	float scale;      /* set the gain of the rms level value, [0 to scale], default: 1.0f */

}hy_crms_param;

/**
*@fn     hy_crms_param_set
*@brief  audio RMS module param set
*
*   audio RMS module param set
*
*@param  void *phy_crms_                 -[in]   Audio RMS structure
*@param  hy_crms_param *pcmrs_param      -[in]   audio RMS param set structure
*/
void hy_crms_param_set(void *phy_crms_, hy_crms_param *pcmrs_param);

/**
*@fn     hy_crms_init
*@brief  crms structure memory allocation and parameter initialization
*
*   crms structure memory allocation and parameter initialization
*
*@return void *phy_crms_                 -[out]   Audio RMS structure
*@param  uint32_t channel                -[in]    channel number
*@param  uint32_t bytenumber             -[in]    audio data byte number 
*/
void * hy_crms_init(uint32_t channel, uint32_t bytenumber);

/**
*@fn     hy_crms_process
*@brief  Audio RMS module Process
*
*   Audio RMS module Process
*
*@return float                                                  -[out]  rms level value
*@param  void * phy_crms_                                       -[in]   Audio RMS module structure
*@param  int8_t *InputAudioStream                               -[in]   input audio stream
*/
float hy_crms_process(void *phy_crms_, char *InputAudioStream);

/**
*@fn     hy_crms_close
*@brief  Audio RMS module close
*
*   Audio RMS module free memory
*
*@return none
*@param  void *phy_crms_                                         -[in]   Audio RMS module structure
*/
void hy_crms_close(void *phy_crms_);

#endif /* CRMS_H */

#ifndef _VOICEDRIVE_H_
#define _VOICEDRIVE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

/**
*@fn     hy_voicedrive_init
*@brief  Audio Drive lip structure memory allocation and parameter initialization
*
*   Audio Drive lip structure memory allocation and parameter initialization
*
*@return void *pst_                      -[out]   Audio Drive lip structure
*@param  float fs                        -[in]    samplerate
*@param  uint32_t bytenumber             -[in]    the length of audio stream process
*/
void *hy_voicedrive_init(uint32_t bytenumber, float fs);
/**
*@fn     hy_voicedrive_set_enable_param
*@brief  set alg enable
*
*   open the alg
*
*@return void *pst_                             -[out]   Audio Drive lip structure
*@param  uint32_t enable                        -[in]    0:bypass,1:enable
*/
void hy_voicedrive_set_enable_param(void *pst_, uint32_t enable);
/**
*@fn     hy_voicedrive_set_scale_param
*@brief  set the gain of rms_level 
*
*   set the gain of rms_level
*
*@return void *pst_                             -[out]   Audio Drive lip structure
*@param  float scale                            -[in]    set the gain of the rms level value, [0 to scale], default: 1.0f 
*/
void hy_voicedrive_set_scale_param(void *pst_, float scale);
/**
*@fn     hy_voicedrive_set_smothcoff_param
*@brief  set the Smoothing of rms_level
*
*   set the Smoothing of rms_level
*
*@return void *pst_                             -[out]   Audio Drive lip structure
*@param  float smoothcoff                       -[in]    the Smoothing value, [0 to 1.0], default: 0.9f
*/
void hy_voicedrive_set_smothcoff_param(void *pst_, float smoothcoff);
/**
*@fn     hy_voicedrive_set_ratio_param
*@brief  set the ratio of drc
*
*   set the compress ratio of drc
*
*@return void *pst_                             -[out]   Audio Drive lip structure
*@param  float ratio                            -[in]    the ratio value, [1.0 to 1000.0], default: 1.0f
*/
void hy_voicedrive_set_ratio_param(void *pst_, float ratio);
/**
*@fn     hy_voicedrive_set_com_ths_param
*@brief  set the compress threshold of drc
*
*   set the compress threshold of drc
*
*@return void *pst_                               -[out]   Audio Drive lip structure
*@param  float com_ths                            -[in]    the compress threshold value, [-20.0f to 0.0f], default: 0.0f
*/
void hy_voicedrive_set_com_ths_param(void *pst_, float com_ths);
/**
*@fn     hy_voicedrive_set_ns_ths_param
*@brief  set the nosie threshold of drc
*
*   set the nosie threshold of drc
*
*@return void *pst_                               -[out]   Audio Drive lip structure
*@param  float ns_ths                             -[in]    the nosie threshold value, [-90.0f to -50.0f], default: -50.0f
*/
void hy_voicedrive_set_ns_ths_param(void *pst_, float ns_ths);
/**
*@fn     hy_voicedrive_set_attacktime_param
*@brief  set the attacktime of drc
*
*   set the attacktime of drc
*
*@return void *pst_                               -[out]   Audio Drive lip structure
*@param  float attacktime                         -[in]    the attacktime value, [0.5f to 100.0f], default: 5.0f
*/
void hy_voicedrive_set_attacktime_param(void *pst_, float attacktime);
/**
*@fn     hy_voicedrive_set_decaytime_param
*@brief  set the decaytime of drc
*
*   set the decaytime of drc
*
*@return void *pst_                               -[out]   Audio Drive lip structure
*@param  float decaytime                          -[in]    the decaytime value, [5.0f to 1000.0f], default: 40.0f
*/
void hy_voicedrive_set_decaytime_param(void *pst_, float decaytime);
/**
*@fn     hy_voicedrive_process
*@brief  Audio Drive lip module Process
*
*   Audio Drive lip module Process
*
*@return float                                                  -[out]  rms level value
*@param  void * pst_                                            -[in]   Audio Drive lip module structure
*@param  char *InputAudioStream                                 -[in]   input audio stream
*@param  uint32_t InputAudioStream_lenght                       -[in]   input audio stream length
*/
float hy_voicedrive_process(void *pst_, char *InputAudioStream, uint32_t InputAudioStream_lenght);
/**
*@fn     hy_voicedrive_close
*@brief  Audio Drive lip module close
*
*   Audio Drive lip module free memory
*
*@return none
*@param  void *hy_voicedrive_close             -[in]   Audio Drive lip module structure
*/
void hy_voicedrive_close(void *pst_);

#ifdef __cplusplus
}
#endif

#endif   /* _VOICEDRIVE_H_ */


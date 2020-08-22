/*
 * Obtained from Embedded Artirst who provided this under BSD license.
 * The Clear BSD License
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PCA6416_H_
#define _PCA6416_H_

#include "fsl_common.h"

/*!
 * @addtogroup pca6416
 * @{
 */


#define PCA_BT_REG_ON_1V8     (1<<0)
#define PCA_WL_REG_ON_1V8     (1<<1)
#define PCA_BT_DEV_WAKE_1V8   (1<<3)
#define PCA_DIR_BT_AUD_WS     (1<<6)
#define PCA_DIR_BT_AUD_CLK    (1<<7)
#define PCA_DIR_WL_GPIO1_DEV_WAKE   (1<<9)
#define PCA_AUDIO_CODEC_CTRL1 (1<<10)
#define PCA_AUDIO_CODEC_CTRL2 (1<<11)
#define PCA_VBAT_VSEL         (1<<14)

#define PCA_ALL_PINS  (PCA_BT_REG_ON_1V8|PCA_WL_REG_ON_1V8|PCA_BT_DEV_WAKE_1V8|PCA_DIR_BT_AUD_WS|PCA_DIR_BT_AUD_CLK|PCA_DIR_WL_GPIO1_DEV_WAKE|PCA_AUDIO_CODEC_CTRL1|PCA_AUDIO_CODEC_CTRL2|PCA_VBAT_VSEL)

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initializes with all pins LOW outputs
 *
 */
status_t PCA6416_Init(void);

/*!
 * @brief Configure pins
 *
 * @param values pins marked with a 1 in the mask will be HIGH - all other pins LOW
 */
status_t PCA6416_SetAllPins(uint16_t values);

/*!
 * @brief Sets each masked pin to LOW
 *
 * @param mask with 1 for each pin that should be LOW
 */
status_t PCA6416_ClearPins(uint16_t mask);

/*!
 * @brief Sets each masked pin to HIGH
 *
 * @param mask with 1 for each pin that should be HIGH
 */
status_t PCA6416_SetPins(uint16_t mask);

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PCA6416_H_ */

/*******************************************************************************
 * EOF
 ******************************************************************************/

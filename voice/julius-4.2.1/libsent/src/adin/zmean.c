/**
 * @file   zmean.c
 *
 * <JA>
 * @brief  入力音声の直流成分の除去
 *
 * 直流成分の除去を行ないます．
 * 
 * 直流成分の推定は入力デバイスごとに異なります．
 * ファイル入力では，データ全体の振幅の平均が用いられます．
 * マイク入力やネットワーク入力の場合，入力ストリームの
 * 最初の @a ZMEANSAMPLES 個のサンプルの平均から計算され，
 * その値がその後の入力に用いられます．
 * </JA>
 * <EN>
 * @brief  Remove DC offset from input speech
 *
 * These function removes DC offset from input speech, like the ZMEANSOURCE
 * feature in HTK.
 *
 * The estimation method of DC offset depends on the type of input device.
 * On file input, the mean of entire samples is used as estimated offset.
 * On microphone input and network input, The first @a
 * ZMEANSAMPLES samples in the input stream are used to estimate the offset,
 * and the value will be used for the rest of the input.
 * </EN>
 *
 * @author Akinobu LEE
 * @date   Sun Feb 13 20:31:23 2005
 *
 * $Revision: 1.3 $
 * 
 */
/*
 * Copyright (c) 1991-2011 Kawahara Lab., Kyoto University
 * Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
 * Copyright (c) 2005-2011 Julius project team, Nagoya Institute of Technology
 * All rights reserved
 */

#include <sent/adin.h>

//#define DBG 1
#ifdef DBG
static int num_its = 0;
#endif

static int zlen = 0;		///< Current recorded length for DC offset estimation
static float zmean = 0.0;	///< Current mean

///< history of prior samples for dynamic level triggering
static SP16 *history;// = (unsigned int *)mymalloc(HISTORY_LEN * sizeof(SP16));
static int history_index;    ///< current location in the history array
static float history_average;

static int zmean_init = 0;

static float max_val_avg;
static int *max_vals;
static int max_val_index = 0;

#define NUM_MAX_VALS 25

/** 
 * Reset status.
 * 
 */
void
zmean_reset()
{
  int i = 0;
  zlen = 0;
  zmean = 0.0;

  if(zmean_init == 0) {
    jlog("initing zmean stuff");
    history = (SP16 *)mymalloc(ZMEANSAMPLES * sizeof(SP16));
    for (i = 0;i < ZMEANSAMPLES; i++) {
      history[i] = 0;
    }
    history_index = 0;
    history_average = 0;

    //scaling params
    max_vals = (int *)mymalloc(NUM_MAX_VALS * sizeof(int));
    for (i = 0;i < NUM_MAX_VALS; i++) {
      max_vals[i] = 0;
    }
    max_val_avg = 0.0;
  }
    zmean_init++;
}

/** 
 * @brief Remove DC offset.
 *
 * The DC offset is estimated by the first samples after zmean_reset() was
 * called.  If the first input segment is longer than ZMEANSAMPLES, the
 * whole input is used to estimate the zero mean.  Otherwise, the zero mean
 * will continue to be updated until the read length exceed ZMEANSAMPLES.
 * 
 * @param speech [I/O] input speech data, will be subtracted by DC offset.
 * @param samplenum [in] length of above.
 * 
 */
void
sub_zmean(SP16 *speech, int samplenum)
{
    int i, max_value = 0;
    int tmp = 0;
    float d;

    for ( i = 0; i < samplenum; i++) {
        /* adjust running average for new value and removal of first value */
        history_average -= history[history_index] / (float) ZMEANSAMPLES;
        history_average += abs(speech[i]) / (float) ZMEANSAMPLES ;

        /* update history */
        history[history_index++] = abs(speech[i]);
        if( history_index == ZMEANSAMPLES ) history_index = 0;
    }

    /* max value */
    for ( i = 0; i < ZMEANSAMPLES; i++) {
        if(history[i] > max_value) max_value = history[i];
    }
    //calculate max values
    max_val_avg -= max_vals[max_val_index] / (float) NUM_MAX_VALS;
    max_val_avg += max_value / (float) NUM_MAX_VALS;
    max_vals[max_val_index++] = max_value;
    if (max_val_index == NUM_MAX_VALS) max_val_index = 0;
    
    

    for (i=0;i<samplenum;i++) {
        d = (float)speech[i] - max_val_avg;
//        d = (float)speech[i] - history_average * SCALE_VAL;
        /* clip overflow */
        if (d < -32768.0) d = -32768.0;
        if (d > 32767.0) d = 32767.0;
        /* round to SP16 (normally short) */
        if (d > 0) speech[i] = (SP16)(d + 0.5);
        else speech[i] = (SP16)(d - 0.5);
    }

#ifdef DBG
    if(DBG == 2) {
        jlog("{ %d", history[0]);
        for(i=1; i<ZMEANSAMPLES; i++) {
            jlog(", %hd", history[i]);
        }
        jlog(" }\t");
        jlog("avg = %f\n", history_average);
    } 
    else if(DBG == 1) {
        num_its++;
        if( num_its % 10 == 1){
            jlog("avg = %f\t", history_average);
            jlog("max_in = %d\t", max_value);
            for(i=0;i<ZMEANSAMPLES; i++){
                if(speech[i] > tmp) tmp = speech[i];
            }
            jlog("max_avg = %f\t", max_val_avg);
            jlog("max_out = %hd\n", tmp);
        }
    }
#endif
}

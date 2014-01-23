/**
 * \file
 * Brief description. Longer description.
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __SLIDERSTYLES_H
#define __SLIDERSTYLES_H

static const char *defaultStyle=
" QSlider::groove:horizontal {border: 1px solid #999999; height: 8px; margin: 2px 0;}\
  QSlider::handle:horizontal {background: #ffffff;border: 1px solid #5c5c5c; width: 18px; height: 18px; margin: -10px 0;  border-radius: 3px;}\
  QSlider::groove:vertical {border: 1px solid #999999; width: 8px; margin: 0 2px;}\
  QSlider::handle:vertical {background: #ffffff;border: 1px solid #5c5c5c; height: 18px; width: 18px; margin: 0 -10px;  border-radius: 3px;}\
";
static const char *unsentStyle=
" QSlider::groove:horizontal {border: 1px solid #999999; height: 8px; margin: 2px 0;}\
  QSlider::handle:horizontal {background: #808080;border: 1px solid #5c5c5c; width: 18px; height: 18px; margin: -10px 0;  border-radius: 3px;}\
  QSlider::groove:vertical {border: 1px solid #999999; width: 8px; margin: 0 2px;}\
  QSlider::handle:vertical {background: #808080;border: 1px solid #5c5c5c; height: 18px; width: 18px; margin: 0 -10px;  border-radius: 3px;}\
";
static const char *unackStyle=
" QSlider::groove:horizontal {border: 1px solid #999999; height: 8px; margin: 2px 0;}\
  QSlider::handle {background: url(:images/diag.png);border: 1px solid #5c5c5c; width: 18px; height: 18px; margin: -10px 0;  border-radius: 3px;}\
  QSlider::groove:vertical {border: 1px solid #999999; width: 8px; margin: 0 2px;}\
  QSlider::handle:vertical {background: url(:images/diag.png);border: 1px solid #5c5c5c; width: 18px; height: 18px; margin: 0 -10px;  border-radius: 3px;}\
";
static const char *initStyle=
" QSlider::groove:horizontal {border: 1px solid #999999; height: 8px; margin: 2px 0;}\
  QSlider::handle {background: #808000;border: 1px solid #5c5c5c; width: 18px; height: 18px; margin: -10px 0;  border-radius: 3px;}\
  QSlider::groove:vertical {border: 1px solid #999999; width: 8px; margin: 0 2px;}\
";
static const char *badAckStyle=
" QSlider::groove:horizontal {border: 1px solid #999999; height: 8px; margin: 2px 0;}\
  QSlider::handle {background: url(:images/cross.png);border: 1px solid #5c5c5c; width: 18px; margin: -10px 0;  border-radius: 3px;}\
  QSlider::groove:vertical {border: 1px solid #999999; width: 8px; margin: 0 2px;}\
  QSlider::handle:vertical {background: url(:images/cross.png);border: 1px solid #5c5c5c; width: 18px; height: 18px; margin: 0 -10px;  border-radius: 3px;}\
";



#endif /* __SLIDERSTYLES_H */

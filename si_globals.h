

// destination for the resized images: either absolute path, or relative to source dir
#define si_settings_dest_dir "."
#define si_settings_output_jpeg_quality 96

// how many images preload when walking through the source directory
#define si_settings_preload_images_min 8
// how many images is too much to handle in memory while walking through t.s.d.
#define si_settings_preload_images_max 15
// how many rescales pre-do when zooming with mouse wheel
#define si_settings_preload_zooms 3
// how many rescales pre-do when in/decreasing brightness
#define si_settings_preload_brightness 3
#define si_settings_preload_gamma 3

// quantity parameters of the effect of mouse wheel zooming
#define si_settings_mousewheel_zoom_base 1.02
#define si_settings_mousewheel_step_base 40

// "next big" image must have height more that this
#define si_settings_big_height 1800

// the target pixel size when marking the resize with keyboard key A (..G rescpectively)
#define si_settings_size_A 800
#define si_settings_size_B 1024
#define si_settings_size_C 1200
#define si_settings_size_D 1600
#define si_settings_size_E 1800
#define si_settings_size_F 1900
#define si_settings_size_G 2400

// initial coords of the program window
#define si_settings_initial_width 1440
#define si_settings_initial_height ((si_settings_initial_width) * 3 / 4)
#define si_settings_initial_x 25
#define si_settings_initial_y 25

// quantity parameters for color changes
#define si_settings_brightness_step 4
#define si_settings_gamma_step 5

#include <QString>
#include <QSize>

// some generally usable functions
extern QString size2string(const QSize & s);
extern bool string2size(const QString & s, QSize * r);
// in sortimg.cpp

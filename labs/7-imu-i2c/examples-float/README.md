### pi with floating point.

If you compile and run the example:

        % cat fp-example.c  
        // trivial example of using floating point + our simple math
        // library.
        #include "rpi.h"
        #include "rpi-math.h"
        void notmain(void) {
            double x = 3.1415;
            printk("hello from pi=%f float!!\n", x);
        
            double v[] = { M_PI, 0, M_PI/2.0, M_PI/2.0*3.0 };
            for(int i = 0; i < 4; i++)  {
                printk("COS(%f) = %f\n", v[i], cos(v[i]));
                printk("sin(%f) = %f\n", v[i], sin(v[i]));
            }
        }

You should get:

        hello from pi=3.141500 float!!
        COS(3.141592) = -1.0
        sin(3.141592) = 0.0
        COS(0.0) = 1.0
        sin(0.0) = 0.0
        COS(1.570796) = 0.0
        sin(1.570796) = 1.0
        COS(4.712388) = -0.0
        sin(4.712388) = -1.0
        DONE!!!


--------------------------------------------------------------------
## Madgewick: where am I?

The directory: `imu/src` has a madgwick implementation to fuse gyro, accel
and mag.  The `madgwick-blake` directory has a version that fuses gyro
and accel.  They give euler angles as well as other location indications.

I didn't have time to figure out how to use this so: great final project!
I'm super interested in how to do this accurately and fast.

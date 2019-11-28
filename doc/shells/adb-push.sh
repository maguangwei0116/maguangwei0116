###### adb push shell for installing targets ######

#!/bin/bash -e

adb push ./targets/*agent* /usr/bin/rt_agent
adb shell chmod +x /usr/bin/rt_agent

adb push ./targets/*monitor* /usr/bin/rt_monitor
adb shell chmod +x /usr/bin/rt_monitor

adb push ./targets/*libcomm.so* /usr/lib/libcomm.so
adb shell chmod +x /usr/lib/libcomm.so

adb push ./shells/start_redtea_app /etc/init.d/start_redtea_app
adb shell chmod +x /etc/init.d/start_redtea_app

adb push ./shells/start_redtea_keep /etc/init.d/start_redtea_keep
adb shell chmod +x /etc/init.d/start_redtea_keep

if [ -e ./targets/test_lpa ]; then
adb push ./targets/test_lpa /data/redtea/test_lpa
adb shell chmod +x /data/redtea/test_lpa
fi

adb shell rm -rf /etc/rc5.d/S99start_redtea_app
adb shell ln -s /etc/init.d/start_redtea_app /etc/rc5.d/S99start_redtea_app

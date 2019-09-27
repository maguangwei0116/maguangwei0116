###### adb push shell for installing targets ######

#!/bin/bash -e

adb push ./targets/*agent* /usr/bin/agent
adb shell chmod +x /usr/bin/agent

adb push ./targets/*monitor* /usr/bin/monitor
adb shell chmod +x /usr/bin/monitor

adb push ./targets/*libcomm.so* /usr/lib/libcomm.so
adb shell chmod +x /usr/lib/libcomm.so

adb push ./shells/start_redtea_app /etc/init.d/start_redtea_app
adb shell chmod +x /etc/init.d/start_redtea_app

adb push ./shells/start_redtea_keep /etc/init.d/start_redtea_keep
adb shell chmod +x /etc/init.d/start_redtea_keep

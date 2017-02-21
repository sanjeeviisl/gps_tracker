#! /bin/sh
# /etc/init.d/vehicle_tracker
#

# Some things that run always
touch /var/lock/vehicle_tracker

# Carry out specific functions when asked to by the system
case "$1" in
  start)
    echo "Starting script vehicle_tracker "
    /root/gpstracker/vehicle_tracker.out
    ;;
  stop)
    echo "Stopping script vehicle_tracker"
    killall -9 vehicle_tracker.out
    ;;
  *)
    echo "Usage: /etc/init.d/vehicle_tracker {start|stop}"
    exit 1
    ;;
esac

exit 0

chmod 755 /etc/init.d/vehicle_tracker
root@skx:~# update-rc.d vehicle_tracker defaults


Edit /etc/rc.local
then 
put
    /root/gpstracker/vehicle_tracker.out
	
	
sudo mv /filename /etc/init.d/
sudo chmod +x /etc/init.d/filename 
sudo update-rc.d filename defaults 

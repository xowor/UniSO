# /bin/sh
for whatever in `ipcs -q | awk '{print $2}'`; do ipcrm -q $whatever; done
for whatever in `ipcs -s | awk '{print $2}'`; do ipcrm -s $whatever; done
for whatever in `ipcs -m | awk '{print $2}'`; do ipcrm -m $whatever; done

pkill auctioneer
pkill client
pkill tao_process
pkill agent

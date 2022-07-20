#!/bin/bash
ASNs=`cat don\'t-scanASN.txt  | awk '{print $1}' | tr '\n' ' '`
mkdir AS
for AS in $ASNs; do
curl https://ipinfo.io/widget/demo/$AS?token=$IPINFOTOKEN -H "Content-Type: application/json" -H "referer: https://ipinfo.io/" > AS/$AS
done
grep -r netblock AS | sed 's/[\"|,]//g' | awk '{ print $3}' | sed 's/.*\:\:.*//g' | awk 'NF' > dcan.ips
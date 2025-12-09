import * as l1 from 'l1parser';

// open l1 profile
let ctx = l1.open();
if (!ctx) {
    print("Failed to open l1profile\n");
    exit(1);
}

// function names here refers to l1util commands

// list
let devs = ctx.list();
printf("%s\n",devs);

// get <dev> <key>
// Note: dev needs to like "MT7981_1_1", can get from list()
if (length(devs) > 0) {
    let dev = devs[0];
    let val = ctx.get(dev, "main_ifname");
    print(dev + " -> main_ifname: " + val + "\n");
}

// if2zone
let zone = ctx.if2zone("ra0");
if (zone) print("ra0 zone: " + zone + "\n");

// zone2if, from 1
let ifs = ctx.zone2if("dev1");
printf("if in zone dev1: %s\n", ifs);

// if2dat
let dat = ctx.if2dat("ra0");
if (dat) print("ra0 dat: " + dat + "\n");

// idx2if, from 1
// l1util idx2if <int>
let ifname = ctx.idx2if(1);
if (ifname) print("Index 1 is: " + ifname + "\n");

// release resource, GC also handles it automatically
ctx.close();


/* results
[ "MT7981_1_1", "MT7981_1_2" ]
MT7981_1_1 -> main_ifname: ra0
ra0 zone: dev1
if in zone dev1: [ "ra0", "ra", "apcli", "wds", "mesh" ]
ra0 dat: /etc/wireless/mediatek/mt7981.dbdc.b0.dat
Index 1 is: ra0
*/
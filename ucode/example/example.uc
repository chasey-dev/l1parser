import * as l1 from 'l1parser';

// open l1 profile
let ctx = l1.open();
if (!ctx) {
    print("Failed to open l1profile\n");
    exit(1);
}

// function names here refers to l1util commands

// list
let devs_str = ctx.list();
print("Devices: " + devs_str + "\n");

// get <dev> <key>
// Note: dev needs to like "MT7981_1_1", can get from list() or cat manually
let devs = split(devs_str, " ");
if (length(devs) > 0) {
    let dev = devs[0];
    let val = ctx.get(dev, "main_ifname");
    print(dev + " -> main_ifname: " + val + "\n");
}

// if2zone
let zone = ctx.if2zone("ra0");
if (zone) print("ra0 zone: " + zone + "\n");

// zone2if
let ifs = ctx.zone2if("lan");
if (ifs) print("lan interfaces: " + ifs + "\n");

// if2dat
let dat = ctx.if2dat("ra0");
if (dat) print("ra0 dat: " + dat + "\n");

// idx2if
// l1util idx2if <int>
let ifname = ctx.idx2if(1);
if (ifname) print("Index 1 is: " + ifname + "\n");

// release resource, GC also handles it automatically
ctx.close();
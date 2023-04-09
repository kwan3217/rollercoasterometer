$fa=1;
$fs=0.4;
module round_rect(w,d,h,r) {
  translate([0,r,0])
      cube([w,d-r*2,h]);
  translate([r,0,0])
      cube([w-r*2,d,h]);
  translate([r,r,0])
      cylinder(h=h,r=r);
  translate([w-r,r,0])
      cylinder(h=h,r=r);
  translate([r,d-r,0])
      cylinder(h=h,r=r);
  translate([w-r,d-r,0])
      cylinder(h=h,r=r);
}

module peg(w,d,h,clip) {
    p0=[0,0];
    p1=[w,0];
    p2=[w,h-clip];
    p3=[w-clip,h];
    p4=[0,h];
    points=[p0,p1,p2,p3,p4];
    translate([0,d,0])
      rotate([90,0,0])
        linear_extrude(height=d)
          polygon(points);
}

pi_w=85;
pi_d=56;
board_h=1.6;
board_clearance=board_h+0.4;
wall_thick=2;
side_tolerance=0.5;
hat_w=65;
hat_d=56.5;


//Pi board
union() {
    color([0,1,0,0.5])
        difference() {
          round_rect(w=pi_w,d=pi_d,h=board_h,r=3);
          translate([3.5,3.5,-1])
            cylinder(h=board_h+2,r=1.35);
          translate([58+3.5,3.5,-1])
            cylinder(h=board_h+2,r=1.35);
          translate([3.5,pi_d-3.5,-1])
            cylinder(h=board_h+2,r=1.35);
          translate([58+3.5,pi_d-3.5,-1])
            cylinder(h=board_h+2,r=1.35);
        }
    //Power LED
    translate([0.31,7,board_h])
        color([1,0,0])
            cube([1.54-0.38,8.8-7,1]);
    //Activity LED
    translate([0.31,10.6,board_h])
        color([0,1,0])
            cube([1.54-0.38,8.8-7,1]);
    //Ethernet jack
    translate([66.5,38,board_h])
        color([0.5,0.5,0.5])
            cube([21.5,15.5,13.5]);
    //USB 3.0 jack
    translate([66.5,20.5,board_h])
        color([0.5,0.5,1.0])
            cube([21.5,13.0,16.0]);
    //USB 2.0 jack
    translate([66.5,2.5,board_h])
        color([0.1,0.1,0.1])
            cube([21.5,13.0,16.0]);
    //Power jack
    translate([6.8,-1.25,board_h])
        color([0.5,0.5,0.5])
            cube([8.8,7.75,3.2]);
    //Micro HDMI 0
    translate([22.4,-1.25,board_h])
        color([0.5,0.5,0.5])
            cube([8.8,7.2,3.0]);
    //Micro HDMI 1
    translate([35.9,-1.25,board_h])
        color([0.5,0.5,0.5])
            cube([8.8,7.2,3.0]);
    //Headphone Jack
    translate([50.5,-2.5,board_h])
        color([0.1,0.1,0.1])
            cube([7,15,6]);
    //MicroSD
    translate([-3,22,-1.57])
        color([1,0.5,0.5])
            cube([15.4,12,1.57]);
    
        
        
}

//standoffs
//cylinder(

//Hat
translate([0,0,12+board_h])
union() {
    color([0.5,0,1,0.5])
        difference() {
          round_rect(w=hat_w,d=hat_d,h=board_h,r=3);
          translate([3.5,3.5,-1])
            cylinder(h=board_h+2,r=1.35);
          translate([58+3.5,3.5,-1])
            cylinder(h=board_h+2,r=1.35);
          translate([3.5,pi_d-3.5,-1])
            cylinder(h=board_h+2,r=1.35);
          translate([58+3.5,pi_d-3.5,-1])
            cylinder(h=board_h+2,r=1.35);
        }
}

jack_clearance=0.5;
groove_d=1;
groove_h=1;

difference() {
  //exterior
  translate([-wall_thick,-wall_thick-side_tolerance,-5])
    round_rect(w=pi_w+side_tolerance+3*wall_thick,
               d=pi_d+side_tolerance+2*wall_thick,
               h=10,r=2+wall_thick);
  //main well
  translate([0,-side_tolerance,-5+wall_thick])
    round_rect(w=pi_w+side_tolerance,d=pi_d+side_tolerance,h=10,r=2);
  //MicroSD cutout
  translate([-5,21,-10])
    cube([10,14,11]);
  //Power cutout
  translate([6.8-jack_clearance,-wall_thick-1,board_h-jack_clearance])
    cube([8.8+2*jack_clearance,wall_thick+2,3.2+2*jack_clearance]);
  //Micro HDMI 0
  translate([22.4-jack_clearance,-wall_thick-1,board_h-jack_clearance])
    cube([8.8+2*jack_clearance,wall_thick+2,3.0+2*jack_clearance]);
  //Micro HDMI 1
  translate([35.9-jack_clearance,-wall_thick-1,board_h-jack_clearance])
    cube([8.8+2*jack_clearance,wall_thick+2,3.0+2*jack_clearance]);
  //Headphone Jack
  translate([50.5-jack_clearance,-wall_thick-1,board_h-jack_clearance])
    cube([7+2*jack_clearance,wall_thick+2,6+2*jack_clearance]);
  //USB 2.0
  translate([pi_w-1,2.5-jack_clearance,board_h-jack_clearance])
    cube([wall_thick*2+2,13+2*jack_clearance,16+2*jack_clearance]);
  //USB 3.0
  translate([pi_w-1,20.5-jack_clearance,board_h-jack_clearance])
    cube([wall_thick*2+2,13+2*jack_clearance,16+2*jack_clearance]);
  //Ethernet
  translate([pi_w-1,38-jack_clearance,board_h-jack_clearance])
    cube([wall_thick*2+2,15.5+2*jack_clearance,13.5+2*jack_clearance]);
  //Power LED
  translate([-wall_thick-1,7,1.6])
    cube([wall_thick*2+2,1.8,1]);
  //Activity LED
  translate([-wall_thick-1,10.6,1.6])
    cube([wall_thick*2+2,1.8,1]);
  //Tab groove
  translate([-groove_d,14,board_clearance])
    cube([pi_w+groove_d*2,
          26,
          groove_h]);
}
    
//MicroSD side lower pegs
translate([-wall_thick/2,21-2,-5]) 
  cube([2+wall_thick/2,2,5]);

translate([-wall_thick/2,21+14,-5]) 
  cube([2+wall_thick/2,2,5]);

//-Power pegs
//translate([7,pi_d-2+wall_thick/2,-5]) 
//  cube([2,2+wall_thick/2,5]);
    
translate([(65+7)/2-1,pi_d-2+wall_thick/2,-5]) 
  cube([2,2+wall_thick/2,5]);
    
translate([65-3-2,pi_d-2+wall_thick/2,-5]) 
  cube([2,2+wall_thick/2,5]);
  
//+Power pegs
translate([7,-wall_thick/2,-5]) 
  cube([2,2+wall_thick/2,5]);
    
translate([(65+7)/2-1,-wall_thick/2,-5]) 
  cube([2,2+wall_thick/2,5]);
    
translate([65-3-2,-wall_thick/2,-5]) 
  cube([2,2+wall_thick/2,5]);
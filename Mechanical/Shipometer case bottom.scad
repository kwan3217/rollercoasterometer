$fa=1;
$fs=0.4;
mm_per_inch=25.4;
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
groove_tolerance=0.5;
hat_w=65;
hat_d=56.5;
fan_w=7;
fan_h=30;
fan_d=30;
jack_clearance=0.5;
groove_d=1.2;
groove_h=1.2;

module tab(w,d,h) {
    union() {
        cube([w,d,h]);
        translate([0,0,groove_h/2])
        rotate([-90,0,0])
        scale([(groove_d-side_tolerance)*2/groove_h,1,1])
        cylinder(h=d,r=groove_h/2);
    }
}

module contents() {
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
    //camera socket
    translate([1.4,16.6,board_h])
        color([1,1,1])
        cube([4,22.6,5.5]);
    //gpio
    translate([32.5-2.54*10,52.5-2.54,board_h])
        color([0.1,0.1,0.1])
        cube([20*2.54,2*2.54,8.5]);
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
/*
//standoffs
translate([3.5,3.5,board_h])
cylinder(r=4.625/sqrt(3),h=12,$fn=6);
translate([58+3.5,3.5,board_h])
cylinder(r=4.625/sqrt(3),h=12,$fn=6);
translate([3.5,pi_d-3.5,board_h])
cylinder(r=4.625/sqrt(3),h=12,$fn=6);
translate([58+3.5,pi_d-3.5,board_h])
cylinder(r=4.625/sqrt(3),h=12,$fn=6);

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

//GPS board
translate([1.004,2.462,board_h+12+board_h+10.5])
union() {
    color([1,0,0,0.5])
    difference() {
        cube([2.0*mm_per_inch,1.7*mm_per_inch,0.8]);
        translate([0.1*mm_per_inch,0.1*mm_per_inch,-1])
            cylinder(h=2.8,r=0.06*mm_per_inch);
        translate([1.9*mm_per_inch,0.1*mm_per_inch,-1])
            cylinder(h=2.8,r=0.06*mm_per_inch);
        translate([0.1*mm_per_inch,1.6*mm_per_inch,-1])
            cylinder(h=2.8,r=0.06*mm_per_inch);
        translate([1.9*mm_per_inch,1.6*mm_per_inch,-1])
            cylinder(h=2.8,r=0.06*mm_per_inch);
    }
    color([0.5,0.5,0.5,1])
    translate([-1.5,8,0.8])
    cube([7.5,8.5,3]);
}
//Fan
  color([0,1,1,1])
  translate([-fan_w-side_tolerance-wall_thick,(pi_d-fan_d-side_tolerance)/2,1])
    cube([fan_w,fan_d,fan_h]);
*/
}

stack_h=board_h+12+board_h+10.5+0.8+3;


module full_case() {

difference() {
  //exterior
  translate([-wall_thick-fan_w-2*side_tolerance-wall_thick,-wall_thick-side_tolerance,-5])
    round_rect(w=pi_w+side_tolerance+3*wall_thick+fan_w+2*side_tolerance+wall_thick,
               d=pi_d+0.5+side_tolerance+2*wall_thick,
               h=5+1+fan_h+side_tolerance*2+wall_thick,r=2+wall_thick);
  //main well
  translate([0,-side_tolerance,-5+wall_thick])
    round_rect(w=pi_w+side_tolerance,d=pi_d+side_tolerance,h=fan_h+1+side_tolerance*2+5-wall_thick,r=2);
  //upper well
  translate([0,-side_tolerance,2])
    round_rect(w=pi_w+side_tolerance,d=pi_d+side_tolerance*2,h=fan_h+1+side_tolerance*2-2,r=2);
    
  //MicroSD cutout
  translate([-15,21,-10])
    cube([20,14,11]);
  //Power cutout
  translate([6.8-jack_clearance,-wall_thick-1,board_h-jack_clearance])
    cube([8.8+2*jack_clearance,wall_thick+2,3.2+2*jack_clearance]);
  //Micro HDMI 0 cutout
  translate([22.4-jack_clearance,-wall_thick-1,board_h-jack_clearance])
    cube([8.8+2*jack_clearance,wall_thick+2,3.0+2*jack_clearance]);
  //Micro HDMI 1 cutout
  translate([35.9-jack_clearance,-wall_thick-1,board_h-jack_clearance])
    cube([8.8+2*jack_clearance,wall_thick+2,3.0+2*jack_clearance]);
  //Headphone Jack cutout
  translate([50.5-jack_clearance,-wall_thick-1,board_h-jack_clearance])
    cube([7+2*jack_clearance,wall_thick+2,6+2*jack_clearance]);
  //USB 2.0/3.0 cutout
  translate([pi_w-11,0,board_h-jack_clearance])
    cube([wall_thick*2+12,35.75,16+2*jack_clearance]);
  //Ethernet cutout
  translate([pi_w-11,35.75,board_h-jack_clearance])
    cube([wall_thick*2+12,pi_d-35.75,13.5+2*jack_clearance]);
  //Tab groove
  color([1,0,0,1])
    translate([-8-groove_tolerance,-groove_tolerance-groove_d,32-5])
      cube([3+groove_tolerance*2,
            5,
            groove_h+groove_tolerance]);
  color([1,1,0,1])
    translate([-8-groove_tolerance,pi_d+0.5-1,32-5])
      cube([3+groove_tolerance*2,
            groove_d+1+groove_tolerance,
            groove_h+groove_tolerance]);
  //Tab groove
  color([0.5,0,1,1])
    translate([pi_w-5-groove_tolerance,-side_tolerance-groove_d,32-5])
      cube([3+groove_tolerance*2,
            pi_d+0.5+side_tolerance+groove_d*2,
            groove_h+groove_tolerance]);
  //Fan cutout
  translate([-fan_w-side_tolerance*2-wall_thick,(pi_d-fan_d-2*side_tolerance)/2,1])
    cube([fan_w+2*side_tolerance,fan_d+2*side_tolerance,fan_h+2*side_tolerance]);
  //MicroSD cutout
  translate([-fan_w-side_tolerance*2-wall_thick,21,-10])
    cube([fan_w+2*side_tolerance,14,21]);  
  //Fan pocket cutout
  translate([-fan_w-side_tolerance*2-wall_thick,-side_tolerance,wall_thick-5])
    cube([fan_w+2*side_tolerance,(pi_d-fan_d-2*side_tolerance)/2-wall_thick,
    3+1+fan_h+2*side_tolerance]);
  //Fan pocket cutout
  translate([-fan_w-side_tolerance*2-wall_thick,
  (pi_d-fan_d-2*side_tolerance)/2+fan_d+2*side_tolerance+wall_thick,wall_thick-5])
    cube([fan_w+2*side_tolerance,(pi_d-fan_d-2*side_tolerance)/2-wall_thick,
    3+1+fan_h+2*side_tolerance]);
//Kwan fan hole
translate([-5,pi_d/2,1+15])
rotate([0,0,-90])
rotate([90,0,0])
scale([28/4,28/4,20])
difference() {
    cylinder(h=1,r=2);
    translate([0,0,-1])
    linear_extrude(height=3)
    polygon([[-1.1875,1.75],
            [-0.25,2.0625],
            [-0.625,0.75],
            [-0.3125,0.75],
            [-0.6875,-0.0625],
            [0.5,0.75],
            [0.25,0.75],
            [1.3125,1.75],
            [2.5,1.75],
            [0.5625,0.0625],
            [1.9375,-2.5],
            [0.9375,-2.5],
            [-0.0625,-0.5],
            [-0.9375,-1.1875],
            [-1.125,-2.5],
            [-1.75,-1]]);
}
//Inner fan hole
translate([9,pi_d/2,1+15])
rotate([0,0,-90])
rotate([90,0,0])
scale([28/4,28/4,20])
    cylinder(h=1,r=2);
translate([-5,pi_d/2-1,1+15])
cube([20,2,16]);
//translate([-5,14,15])
//cube([30/2,30/2,30/2])
  //upper vent
  translate([30,(pi_d-30)/2,30])
  intersection() {
    cube([40,30,30]);
    union() {
      for(i=[-6:1:8]) {
        translate([i*5,0,0])
        rotate([0,0,-45])
        cube([2.5*sin(45),60,30]);
      }
    }
  }
  //SMA jack hole
  translate([60,-10,25])
  rotate([0,0,90])
  rotate([0,90,0])
  cylinder(h=10,r=(6.6+side_tolerance)/2);
  translate([60,-wall_thick-side_tolerance+1.2,25])
  rotate([0,0,90])
  rotate([0,90,0])
  cylinder(h=1.8,r=4.5+0.5,$fn=6);
  
  //clearance for GPS USB
  translate([-1,9.5,0])
  cube([2,11,30]);
  
  //Shipometer text
  translate([-10,47,33])
  linear_extrude(2)
  text(text="Shipometer 23.04",size=7,font="Verdana:style=Bold");
  translate([-10,3,33])
  linear_extrude(2)
  text(text="stkwans.blogspot.com",size=5.5,font="Verdana:style=Bold");
/*
  //Disney Cruise Logo
  translate([0,-wall_thick-side_tolerance+1.2,10])
  rotate([90,0,0])
  linear_extrude(2)
  import("disney cruise logo.svg");
*/
/*
  //Disney Dream
  translate([70,pi_d+0.5+wall_thick-0.8,-2.5])
  rotate([0,0,180])
  rotate([90,0,0])
  scale(1.2)
  linear_extrude(2)
  import("Disney_Dream.svg");
*/
}

//MicroSD side lower pegs
translate([-wall_thick/2,21-2,-5]) 
  cube([2+wall_thick/2,2,5]);

translate([-wall_thick/2,21+14,-5]) 
  cube([2+wall_thick/2,2,5]);


//-Power pegs
//translate([7,pi_d-2+wall_thick/2,-5]) 
//  cube([2,2+wall_thick/2,5]);
    
translate([(65+7)/2-1,pi_d-3+wall_thick/2,-5]) 
  cube([2,2+wall_thick/2,5]);
    
translate([65-3-2,pi_d-3+wall_thick/2,-5]) 
  cube([2,2+wall_thick/2,5]);
  
//+Power pegs
translate([7,-wall_thick/2,-5]) 
  cube([2,2+wall_thick/2,5]);
    
translate([(65+7)/2-1,-wall_thick/2,-5]) 
  cube([2,2+wall_thick/2,5]);
    
translate([65+2-2,-wall_thick/2,-5]) 
  cube([2,2+wall_thick/2,5]);
  
//-MicroSD side lower pegs
translate([pi_w-2,21-4,-5]) 
  cube([2+wall_thick/2,2,5]);

translate([pi_w-2,21+14,-5]) 
  cube([2+wall_thick/2,2,5]);
  
  //-MicroSD side upper walls
  translate([pi_w-10,18-0.4,board_clearance])
  cube([11+wall_thick+1.5,0.8,1+fan_h+side_tolerance*2]);
  translate([pi_w+side_tolerance,18-0.4,0])
  cube([wall_thick+2,0.8,10]);

  translate([pi_w-10,35.75-0.4,board_clearance])
  cube([11+wall_thick+1.5,0.8,1+fan_h+side_tolerance*2]);
  translate([pi_w+side_tolerance,35.75-0.4,0])
  cube([wall_thick+2,0.8,10]);
  
}

module separation_plane() {
  union() {
    translate([-100,-100,32])
    cube([300,300,100]);
    translate([pi_w-11,0,board_h-jack_clearance])
    cube([20,pi_d,100]);
  }
}

module case_bottom() {
  difference() {
    full_case();
    translate([0,0,-1e-6])
    separation_plane();
    //translate([-100,-100,-100])
    //cube([300,300,120]);
    translate([-100,-100,-100])
    cube([90+pi_w,300,300]);

  }
}

module case_top() {
  intersection() {
    full_case();
    translate([0,0,1e-6])
    separation_plane();
  }

  color([1,0,0])
  translate([pi_w-5+3,0,32-5])
  rotate([0,0,90])
  tab(3,3,5);

  color([1,1,0])
  translate([pi_w-5,pi_d+0.5-side_tolerance,32-5])
  rotate([0,0,-90])
  tab(3,3,5);
  
  translate([-8+3,0,32-5])
  rotate([0,0,90])
  tab(3,3,5);

  color([0,0,1])
  translate([-8,pi_d+0.5-side_tolerance*2,32-5])
  rotate([0,0,-90])
  tab(3,3,5);
  
  color([1,0,1])
  translate([-wall_thick-fan_w-1+side_tolerance,7-side_tolerance,30])
  cube([fan_w,3,3]);

  color([0,1,1])
  translate([-wall_thick-fan_w-1+side_tolerance,45.5+side_tolerance,30])
  cube([fan_w,3,3]);
}

//contents();
//case_bottom();
case_top();


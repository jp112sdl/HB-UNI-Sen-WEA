//LTB weather in 3D
//Bucket tipping rain-guage

//Non-Printable Extras
//Neodymium Magnet, cylinder  2mm thick, 3mm diam
//Red Switch, 14mm long 2mm diam
//Rod for the axle 1.5 mm OD, rescued brass antenna tubing is ideal!
//2 rings of plastic tubing to put on opposite ends of tipping bucket axle.

$fn=60;
hy=5;  //hysteresis
//Main support base for bucket, drainage and electronics
module Rbase(){
	difference(){
		color("green") union(){
			cylinder(4,50,50); //main base
			cylinder(8,45,45); //outer base rim
			translate([0,0,4]) cylinder(4,40,40); //main middle base
			translate([0,0,20]) cylinder(40,45,45,true); //main base outer cylinder
		}
		translate([0,0,25]) cylinder(40,43,43,true); //main base inner cylinder
		translate([30,26,0]) cylinder(20,2,2,true);  //base mounting/levelling hole
		translate([-30,26,0]) cylinder(20,2,2,true); //base mounting/levelling hole
		translate([0,-40,0]) cylinder(20,2,2,true);  //base mounting/levelling hole
		rotate([0,0,90]) cylinder(10,2.5,2.5,true);  //small water drainhole
		translate([0,30,0]) rotate([0,0,90]) cylinder(10,2.5,2.5,true);  //small wire exit hole
		translate([0,40,0]) rotate([0,90,90]) cylinder(20,2.5,2.5,true);  //small wire exit tunnel
	}
}
module Rffsupport(){
	color("blue")
	difference(){
		translate([0,0,20]) cube([82.5,4,40],true); //side support
		translate([0,0,23+hy]) rotate([90,0,0]) cylinder(5,2,2,true);//locating semi-circle
	}
}

module drainramps(){
	//drain the water our of the central hole
	color("yellow") translate([-23,0,8]) rotate([0,10,0])cube([39.5,30,1],true);
	color("yellow") translate([23,0,8]) rotate([0,-10,0])cube([39.5,30,1],true);
	//projections to stop lip of bucket being held by water tension
	color("red") translate([-38,0,11]) rotate([45,10,0])cube([10,2,2],true);
	color("red") translate([38,0,11]) rotate([45,-10,0])cube([10,2,2],true);
	
}

module Rrelaysupport(){
	//place the reed switch in the gap and heat the wires on it to melt into the 
	//plastic and hold it in place, or just glue it there.
	color("orange")
	difference(){
		translate([0,30,50]) cube([30,4,20],true); //Plate for mounting reed switch
		translate([0,30,50]) rotate([0,90,0]) cube([15,5,3],true); //hole for reed switch
	}
}

module Rmainbase(){
	difference(){
		Rbase();
	  translate([0,0,30]) cube([82.4,124,40],true);//big cutaway
	}
		translate([0,-16,0]) Rffsupport();
		translate([0,+16,0]) Rffsupport();
		translate([0,-14,0]) Rrelaysupport();
		drainramps();
}

//Flip Flop tipping bucket
module Rflipflop(){
	difference(){
		union(){
			difference(){
				//begin with
				translate([0,0,10]) color("red") cube([80,25,20],true); //main body
				//take these out
				translate([-20,0,35]) rotate([0,155,0]) cube([80,25,40],true); 
				translate([20,0,35]) rotate([0,-155,0]) cube([80,25,40],true); //subtract top half of cube
				translate([0,0,10]) color("red") cube([80,23,20],true); //main body
			}
		//add these in
		cube([80,25,1],true);	//the flat base of tipping cups
		color("gray")  translate([0,0,8]) cube([2,25,25],true); //divider plate
		translate([0,0,-hy]) rotate([90,0,0]) cylinder(25,6,6,true); //outer axle bearing
		translate([0,13,-hy])  rotate([90,0,0]) cylinder(1,3,3,true); //washer A
		translate([0,-13,-hy])  rotate([90,0,0]) cylinder(1,3,3,true); //washer B
		translate([0,13,-hy])  rotate([90,0,0]) cylinder(1,3,5,true); //washer A slope
		translate([0,-13,-hy])  rotate([90,0,0]) cylinder(1,5,3,true); //washer B slope
		color("grey")	translate([0,12,17]) rotate([0,90,0])cylinder(6,3,3,true); //support for the magnet	
			}
	//take these out	
	translate([0,0,-hy]) rotate([90,0,0]) cylinder(29,2,2,true); //inner axle
	translate([-5,12.5,12.5],true) cube([10,10,10]); //removes excess magnet support
	translate([0,12,17]) cube([4,3,2],true);//magnet hole, NB N-S is parallel to the reed switch, ie field lines run along the reed switches
		
	}
}

module funnel(){
	translate([0,0,5])
		difference(){
			color("red") cylinder(90,50,50); //main base
			cylinder(90,45,45); //main base
		}
	difference(){
		translate([0,0,55]) cylinder(40,2,48);
		translate([0,0,58]) cylinder(40,2,48);
		translate([0,0,55]) cylinder(10,2,2);
	}
}

//Main parts
Rmainbase();
translate([0,0,38-hy])	rotate([0,0,0]) Rflipflop(); //flip flop tipping
//Rflipflop();
difference(){
	//funnel();
	//uncomment for a "demonstration funnel" to show people how it works
	//rotate([0,0,90]) translate([10,-100,0]) cube([100,200,100]);  //for demo cutaway
}
//LTB Weather in 3D
//Anemometer and Wind Direction

$fn=30;
//Non-Printable Extras
//Neodymium Magnet, cylinder  2mm thick, 3mm diam
//Red Switches, 14mm long 2mm diam
//Thrust bearings, 8mm OD, 5 mmID, 2.5mm thick
//Rods for the bearings 5mm OD, rescued brass antenna tubing is ideal!
//Heat shrink tubing to make a small sleeve to adjust height off bearings

//Make the shape of the front of the vane
module prism(l, w, h){
  polyhedron(
    points=[[0,0,0], [l,0,0], [l,w,0], [0,w,0], [0,w,h], [l,w,h]],
    faces=[[0,1,2,3],[5,4,3,2],[0,4,5,1],[0,3,4],[5,2,1]]
   );
}

module windvane(){
	difference(){
		union(){
			rotate([0,90,0]) cylinder(150,5,5,true); //main rod
			translate([-82.5,0,0])rotate([0,90,0]) cylinder(15,0,5,true); //pointy tip
			translate([45,-1,0]) cube([30,2,40]); //vane_1
			rotate([0,0,-90])  translate([-1,0,5]) prism(2,45, 35);//vane_2
			translate([0,0,-10]) cylinder(30,20,20,true); //mounting bracket
		}
		//This next dimension hollows the innner skirt of the windvane
		//It controls how close you can get the magnet to the reed switches
		//The magnet needs N-S to be vertical aligned with the reed switches
		//If it is not close enough or strong enough it will not reliably
		//trigger the both reed switches when positioned halfway between two.
		//This effect allows 16 directions to be obtained from 8 switches.
		//You may need to experiment a bit here to make it reliable.
		translate([0,0,-15]) cylinder(30,13.2,13.2,true); //mount cap inner
		cylinder(60,2.5,2.5,true); //mounting spindle axle hole 5mm diam 
		//magnet mount, radial distance is adjustable for multiple reed 'hits'
		//hitting 2 reed switches at a time doubles resolution from 8 to 16
		 translate([0,-11,-15]) rotate([0,0,90]) cube([20,3.1,2.6],true); 
	}
}

module dirbasesupport(){
  difference(){
   union(){
		translate([0,0,2.5]) cylinder(5,10,10,true); //axle mount top
 		color("red") translate([0,0,-15]) cylinder(30,12,12,true); //mount cap inner
	 translate([0,0,-39]) //move bottom skirt
	 difference(){
		 color("red") cylinder(25,25,25,true); //outer skirt
		 translate([0,0,-2]) cylinder(23,23,23,true); //inner skirt
	 }
	}
		cylinder(70,3,3,true); //spindle hole,6mm ID for 5mm axle
	  translate([0,0,3]) cylinder(4,4,4,true); //top hole for bearing
	  translate([0,0,-28]) cylinder(4,4,4,true); //bottom hole for bearing 	 	
		rotate([0,0,22.5]) translate([7,0,-10]) cylinder(60,1,1,true);//Relay common wire
		rotate([90,180,22.5]) translate([-7,-5,0]) cylinder(6,1,1,true);//Relay common wire
 
	   for(rrelay = [0 : 45 : 360])
		  //rotate([90,rrelay,0]) translate([25,10,0]) 
		rotate([0,0,rrelay]) translate([9.5,0,-10]) cylinder(60,2,2,true);//Relays mounts
	 }
}

//Print three of these separately
module cup(){
	color("blue")
	difference(){
		translate([-66,0,0]) //radiate from center
		difference(){
			union(){
				sphere(20); //outer wall
				translate([35,0,5]) cube([50,10,10],true); //support arm
			}
			sphere(17); //inner space
			translate([0,0,-10]) cube([40,40,20],true); //chop off what we don't need
		}
		rotate([90,0,0]) translate([-10,5,0])  cylinder(20,1,1,true); //screw mount
	}
}

module cups3mount(){
	difference(){
	union(){
		cylinder(20,20,20,true);
	 translate([0,0,-19]) //move bottom skirt
	 difference(){
		 cylinder(25,25,25,true); //outer skirt
		 translate([0,0,-2]) cylinder(23,23,23,true); //inner skirt
	 }
 }
		cups3(); //just used to create and align holes for cups
		rotate([0,0,174]) translate([-10,5,0]) cylinder(40,1.5,1.5,true); //screw mount
		rotate([0,0,54]) translate([-10,5,0]) cylinder(40,1.5,1.5,true); //screw mount
		rotate([0,0,294]) translate([-10,5,0]) cylinder(40,1.5,1.5,true); //screw mount
		cylinder(40,2.5,2.5,true);//spindle axle 5mm OD
		translate([0,-8,-10]) cube([2,3,4],true);//magnet hole, N-S is aligned tangential to circumference, ie magnetic force lines are along the switch reeds
		}
}	

//now combine all three into one module, to set up holes in mount base
module cups3(){
	rotate([90,0,0])cup();
	rotate([90,0,120])cup();
	rotate([90,0,240])cup();
}

module cuprotormount(){
	difference(){
		cylinder(30,20,20,true);
		rotate([0,0,54]) translate([-10,5,0]) cylinder(40,1.5,1.5,true); //reed relay mount
		rotate([0,0,294]) translate([-10,5,0]) cylinder(40,1.5,1.5,true); //reed relay mount
		rotate([0,90,57]) translate([-15,5.5,0]) cylinder(20,1.5,1.5,true); //reed relay slot for mounting

		cylinder(40,3,3,true);//6mm ID spindle bearing shaft, for a 5mm axle
	  translate([0,0,14.5]) cylinder(2,4,4,true); //top hole for bearing 8mmOD
	  translate([0,0,-14.5]) cylinder(2,4,4,true); //bottom hole for bearing 8mm OD
	}
}

//Glue this to the side of the speed indicator for a mouning option
module bracketSpeed(){
	difference(){	
		cylinder(10,10,10,true);
		translate([20,0,0]) cylinder(10,20,20,true);
	}
	translate([-13,0,0]) rotate([90,0,90,]) cylinder(10,5,5,true);
}

//Glue this to the side of the direction indicator for a mouning option
module bracketDirection(){
	difference(){	
		cylinder(10,10,10,true);
		translate([25,0,0]) cylinder(10,25,25,true);
	}
	translate([-13,0,0]) rotate([90,0,90,]) cylinder(10,5,5,true);
}

//Use this to mount the aluminium crossbar on a vertical aluminium tube
module Tpiece(){
	difference(){	
		union(){
			cylinder(30,9,9,true);
			translate([-17,0,0]) rotate([90,0,90,]) cylinder(20,5.25,5.25,true);
		}
		translate([0,0,0]) cylinder(30,6.5,6.5,true);
	}
}

//Vertical mount for the aluminium tube onto the baseboard
module Tframebase(){
	difference(){
		union(){
	translate([0,0,2.5]) cylinder(5,20,20,true);
	translate([0,0,10]) cylinder(10,20,5.25,true);
		}
	rotate([0,0,0,]) cylinder(30,6.5,6.5,true);
		
	 rotate([0,0,0]) translate([15,0,10]) cylinder(10,2,2,true);	
	rotate([0,0,120]) translate([15,0,10])cylinder(10,2,2,true);	
	rotate([0,0,240]) translate([15,0,10])cylinder(10,2,2,true);
		
	 rotate([0,0,0]) translate([15,0,10]) cylinder(30,1,1,true);	
	rotate([0,0,120]) translate([15,0,10])cylinder(30,1,1,true);	
	rotate([0,0,240]) translate([15,0,10])cylinder(30,1,1,true);	
}
}

module direction(){
difference(){
  union(){
		translate([0,0,6]) windvane(); //direction indicator
		translate([0,0,0]) dirbasesupport(); //mount for direction indicator
	}
	//translate([-220,0,-200]) cube([400,400,400]); //shows cutaway
 }
}

module speed(){
//cup(); //one cup, print three times
difference(){
union(){ 
	cups3(); //three cups in place, DO NOT print this
  cups3mount(); //mount support for cups
  translate([0,0,-28]) color("gold") cuprotormount(); //speed base and bearings holder
	}
		//translate([-220,0,-200]) cube([400,400,400]); //shows cutaway
}
}

//Rendering visually
//Use these to see the parts "in-situ"

//Wind Speed Parts
translate([-60,0,0]) speed();
//translate([-20,0,0]) bracketSpeed();

//Wind Direction Parts
//translate([60,0,0]) direction();
//translate([5,0,0]) bracketDirection();

//Mounting Frame Parts
//Tpiece();
//Tframebase();

//STL List (render each separately and export as .stl)

//Wind speed parts (Anemometer)
  //cup(); //print 3 of these
  //cups3mount();
  //cuprotormount();
	//bracketSpeed();
//Wind direction parts
	//windvane();
	//dirbasesupport();
	//bracketDirection();
//Frame parts
	//Tpiece();
	//Tframebase();
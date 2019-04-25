//////////////////////////////
//
// example: aluminium block with 5 holes 2D
//          unstructured strategy.
//
/////////////////////////////

SetFactory("OpenCASCADE");

lc = DefineNumber[ 0.1, Name "Parameters/lc" ];

Point(1) = {0, 1, 0, lc};
Point(2) = {0, 0, 0, lc};
Point(3) = {1, 0, 0, lc};
Point(4) = {1, 1, 0, lc};

Line(1) = {1, 2};
Line(2) = {2, 3};
Line(3) = {3, 4};
Line(4) = {4, 1};


//SetFactory("Built-in");
Curve Loop(1) = {1, 2, 3, 4};
Plane Surface(1) = {1};


// extrude 2d surface
Extrude {0, 0, 1} {
    Surface{1}; Layers{1/lc}; Recombine;
}


////+
//Physical Surface("Front") = {7};
////+
//Physical Surface("Back") = {1};
////+
//Physical Surface("Left") = {2};
////+
//Physical Surface("Right") = {5};
////+
//Physical Surface("Bottom") = {4};
////+
//Physical Surface("Top") = {3};
////+
Physical Volume("M1") = {1};
//+

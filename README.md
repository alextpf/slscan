# slscan
The goal is to use affordable, off the shelf, commercial equipment, and write up my own codes to make it work.

The equipment includes: 2 web cams, a projector, and a PC, with the total price of less than $200 ( not including a DIY <a href="https://alexchenwoodworks.blogspot.com/2018/12/pc-controllable-turn-table.html">PC-controllable turn table</a> which was originally intended to be used in this project, but later turned out not usable ).
The software was written from scratch with C++ and OpenCV. The algorithm basically consists of:

- calibrating each of the cameras, 
- calibrating the stereo cameras as a pair, and
- the time-space coding and decoding for finding out the correspondence and  depth map.

The results turned out well, even though not as good as I expected at the beginning, in the sense that there are some distortion in certain direction, most possibly due to the imperfection of my calibration. Therefore the original plan of using the turn table to fuse the views from different angles didn't come true. But, for a single view, the <a href="https://github.com/alextpf/slscan/tree/master/present">result</a> is satisfying enough.



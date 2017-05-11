# fidl

Neuroimaging analysis software

Four directories are included: idl, c, cmake and matlab.  
The user interface is written in the IDL scripting language whose code resides in the idl directory.  
Most of the computing is done in C/C++ whose code resides in the c directory.  
The cmake directory includes CMakeLists.txt for building the libraries and binaries. You will need to edit it.  

fidl calls CiftiLib, see https://github.com/Washington-University/CiftiLib

fidl calls RSCRIPT with scripts written on the fly by the binaries.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;fidl_anova4.c calls the lmerTest library.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;fidl_pcat.c calls the RCUR library.  

Included in the matlab directory is a rather embarrassing plotting program that is compatible with fidl's column oriented text output. It runs on even the most ancient of MATLAB installations.

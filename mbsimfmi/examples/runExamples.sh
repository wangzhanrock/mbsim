#!/bin/bash
BIT=$(getconf LONG_BIT)
rm *.tmp
echo "Start 1st test"
../script/mbsimfmi.sh ../../../../MBSim/mbsim/examples/xmlflat/hierachical_modelling/MBS.mbsimprj.flat.xml 1>/dev/null
../extern/fmuCheck.linux$BIT mbsim.fmu >out_hierachical.tmp 2>&1 #1>&-
mv mbsim.fmu mbsimH.fmu
#rm mbsim.fmu
echo "Hierarchical modeling test completed. See out_hierachical.tmp for results"
echo "Start 2nd test"
../script/mbsimfmi.sh ../../../../MBSim/mbsim/examples/xmlflat/pendulum_with_joints/MBS.mbsimprj.flat.xml 1>/dev/null
../extern/fmuCheck.linux$BIT mbsim.fmu >out_pendulum.tmp 2>&1 #1>&-
mv mbsim.fmu mbsimP.fmu
#rm mbsim.fmu
echo "Pendulum with joints test completed. See out_pendulum.tmp for results"
echo "Start 3rd test"
../script/mbsimfmi.sh 1>/dev/null
../extern/fmuCheck.linux$BIT mbsim.fmu >out_2mass.tmp 2>&1 #1>&-
mv mbsim.fmu mbsim2.fmu
#rm mbsim.fmu
echo "Two mass oscillator test completed. See out_2mass.tmp for results"
echo "Clean folder"
rm ./*.h5
rm ./*.xml
echo "Script completed"

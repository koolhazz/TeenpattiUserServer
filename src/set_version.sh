#!/bin/sh
Version=1.0.0
SubVersion=0

SetRevision()
{
        Revision=`svn info|grep "Last Changed Rev"|awk '{print $4}'`
        SubVersion=${Revision}
        sed -i 's/'#{VERSION}'/'${Version}'/g' Version.c
        sed -i 's/'#{Svn_Revision}'/'${SubVersion}'/g' Version.c
}

SetRevision

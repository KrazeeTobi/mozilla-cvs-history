#! /bin/sh
#
# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is the Network Security Services (NSS)
#
# The Initial Developer of the Original Code is
# Sun Microsystems, Inc.
# Portions created by the Initial Developer are Copyright (C) 2006-2007
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#   Slavomir Katuscak <slavomir.katuscak@sun.com>, Sun Microsystems
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK *****

########################################################################
#
# mozilla/security/nss/tests/memleak/memleak.sh
#
# Script to test memory leaks in NSS
#
# needs to work on Solaris and Linux platforms, on others just print a message
# that OS is not supported
#
# special strings
# ---------------
#   FIXME ... known problems, search for this string
#   NOTE .... unexpected behavior
#
########################################################################

############################# memleak_init #############################
# local shell function to initialize this script 
########################################################################
memleak_init()
{
	if [ -z "${INIT_SOURCED}" -o "${INIT_SOURCED}" != "TRUE" ]; then
		cd ../common
		. ./init.sh
	fi
	
	if [ ! -r ${CERT_LOG_FILE} ]; then
		cd ${QADIR}/cert
		. ./cert.sh
	fi

	SCRIPTNAME="memleak.sh"
	if [ -z "${CLEANUP}" ] ; then
		CLEANUP="${SCRIPTNAME}"
	fi

 	NSS_DISABLE_ARENA_FREE_LIST="1"
 	export NSS_DISABLE_ARENA_FREE_LIST
	
	OLD_LIBRARY_PATH=${LD_LIBRARY_PATH}
	TMP_LIBDIR="${HOSTDIR}/tmp$$"
	TMP_STACKS="${HOSTDIR}/stacks$$"
	
	PORT=${PORT:-8443}
	
	MODE_LIST="NORMAL BYPASS FIPS"	
	
	SERVER_DB="${HOSTDIR}/server_memleak"
	CLIENT_DB="${HOSTDIR}/client_memleak"
	cp -r ${HOSTDIR}/server ${SERVER_DB}
	cp -r ${HOSTDIR}/client ${CLIENT_DB}
	
	LOGDIR="${HOSTDIR}/memleak_logs"
	mkdir -p ${LOGDIR}

	FOUNDLEAKS="${LOGDIR}/foundleaks"
	
	REQUEST_FILE="${QADIR}/memleak/sslreq.dat"
	IGNORED_STACKS="${QADIR}/memleak/ignored"
	
	gline=`echo ${OBJDIR} | grep "_64_"`
	if [ -n "${gline}" ] ; then
		BIT_NAME="64"
	else
		BIT_NAME="32"
	fi
		
	case "${OS_NAME}" in
	"SunOS")
		DBX=`which dbx`
		
		if [ $? -eq 0 ] ; then
			echo "${SCRIPTNAME}: DBX found: ${DBX}"
		else
			echo "${SCRIPTNAME}: DBX not found, skipping memory leak checking."
			exit 0
		fi
		
		PROC_ARCH=`uname -p`
				
		if [ "${PROC_ARCH}" = "sparc" ] ; then
			if [ "${BIT_NAME}" = "64" ] ; then
				FREEBL_DEFAULT="libfreebl_64fpu_3"
				FREEBL_LIST="${FREEBL_DEFAULT} libfreebl_64int_3"
			else
				FREEBL_DEFAULT="libfreebl_32fpu_3"
				FREEBL_LIST="${FREEBL_DEFAULT} libfreebl_32int_3 libfreebl_32int64_3"
			fi
		else
			if [ "${BIT_NAME}" = "64" ] ; then
				echo "${SCRIPTNAME}: OS not supported for memory leak checking."
				exit 0
			fi
			
			FREEBL_DEFAULT="libfreebl_3"
			FREEBL_LIST="${FREEBL_DEFAULT}"
		fi
		
		RUN_COMMAND_DBG="run_command_dbx"
		PARSE_LOGFILE="parse_logfile_dbx"
		;;
	"Linux")
		VALGRIND=`which valgrind`
		
		if [ $? -eq 0 ] ; then
			echo "${SCRIPTNAME}: Valgrind found: ${VALGRIND}"
		else
			echo "${SCRIPTNAME}: Valgrind not found, skipping memory leak checking."
			exit 0
		fi

		if [ "${BIT_NAME}" = "64" ] ; then
			echo "${SCRIPTNAME}: OS not supported for memory leak checking."
			exit 0
		fi
		
		FREEBL_DEFAULT="libfreebl_3"
		FREEBL_LIST="${FREEBL_DEFAULT}"
				
		RUN_COMMAND_DBG="run_command_valgrind"
		PARSE_LOGFILE="parse_logfile_valgrind"
		;;
	*)
		echo "${SCRIPTNAME}: OS not supported for memory leak checking."
		exit 0
		;;
	esac

	if [ "${BUILD_OPT}" -eq "1" ] ; then
		OPT="OPT"
		unset NSS_DISABLE_UNLOAD
	else 
		OPT="DBG"
		NSS_DISABLE_UNLOAD="1"
		export NSS_DISABLE_UNLOAD
	fi

	SELFSERV_ATTR="-D -p ${PORT} -d ${SERVER_DB} -n ${HOSTADDR} -e ${HOSTADDR}-ec -w nss -c ABCDEF:C001:C002:C003:C004:C005:C006:C007:C008:C009:C00A:C00B:C00C:C00D:C00E:C00F:C010:C011:C012:C013:C014cdefgijklmnvyz -t 5"
	TSTCLNT_ATTR="-p ${PORT} -h ${HOSTADDR} -c j -f -d ${CLIENT_DB} -w nss"

	tbytes=0
	tblocks=0
	
	MEMLEAK_DBG=1
	export MEMLEAK_DBG
}

########################### memleak_cleanup ############################
# local shell function to clean up after this script 
########################################################################
memleak_cleanup()
{
	unset MEMLEAK_DBG
	. ${QADIR}/common/cleanup.sh
}

############################ set_test_mode #############################
# local shell function to set testing mode for server and for client
########################################################################
set_test_mode()
{
	if [ "${server_mode}" = "BYPASS" ] ; then
		echo "${SCRIPTNAME}: BYPASS is ON"
		SERVER_OPTION="-B -s"
		CLIENT_OPTION=""
	elif [ "${client_mode}" = "BYPASS" ] ; then
		echo "${SCRIPTNAME}: BYPASS is ON"
		SERVER_OPTION=""
		CLIENT_OPTION="-B -s"
	else
		echo "${SCRIPTNAME}: BYPASS is OFF"
		SERVER_OPTION=""
		CLIENT_OPTION=""
	fi
	
	if [ "${server_mode}" = "FIPS" ] ; then
		modutil -dbdir ${SERVER_DB} -fips true -force
		modutil -dbdir ${SERVER_DB} -list
		modutil -dbdir ${CLIENT_DB} -fips false -force
		modutil -dbdir ${CLIENT_DB} -list
		
		echo "${SCRIPTNAME}: FIPS is ON"
		cipher_list="c d e i j k n v y z"
	elif [ "${client_mode}" = "FIPS" ] ; then
		
		modutil -dbdir ${SERVER_DB} -fips false -force
		modutil -dbdir ${SERVER_DB} -list
		modutil -dbdir ${CLIENT_DB} -fips true -force
		modutil -dbdir ${CLIENT_DB} -list
		
		echo "${SCRIPTNAME}: FIPS is ON"
		cipher_list="c d e i j k n v y z"
	else
		modutil -dbdir ${SERVER_DB} -fips false -force
		modutil -dbdir ${SERVER_DB} -list
		modutil -dbdir ${CLIENT_DB} -fips false -force
		modutil -dbdir ${CLIENT_DB} -list
		
		echo "${SCRIPTNAME}: FIPS is OFF"
		cipher_list="A B C D E F :C001 :C002 :C003 :C004 :C005 :C006 :C007 :C008 :C009 :C00A :C010 :C011 :C012 :C013 :C014 c d e f g i j k l m n v y z"
	fi
}

############################## set_freebl ##############################
# local shell function to set freebl - sets temporary path for libraries
########################################################################
set_freebl()
{
	if [ "${freebl}" = "${FREEBL_DEFAULT}" ] ; then
		LD_LIBRARY_PATH="${OLD_LIBRARY_PATH}"
		export LD_LIBRARY_PATH
	else
		if [ -d "${TMP_LIBDIR}" ] ; then
			rm -rf ${TMP_LIBDIR}
		fi
		mkdir ${TMP_LIBDIR}
		cp ${DIST}/${OBJDIR}/lib/*.so ${DIST}/${OBJDIR}/lib/*.chk ${TMP_LIBDIR}
		
		echo "${SCRIPTNAME}: Using ${freebl} instead of ${FREEBL_DEFAULT}"
		mv ${TMP_LIBDIR}/${FREEBL_DEFAULT}.so ${TMP_LIBDIR}/${FREEBL_DEFAULT}.so.orig
		cp ${TMP_LIBDIR}/${freebl}.so ${TMP_LIBDIR}/${FREEBL_DEFAULT}.so
		mv ${TMP_LIBDIR}/${FREEBL_DEFAULT}.chk ${TMP_LIBDIR}/${FREEBL_DEFAULT}.chk.orig
		cp ${TMP_LIBDIR}/${freebl}.chk ${TMP_LIBDIR}/${FREEBL_DEFAULT}.chk
		
		LD_LIBRARY_PATH="${TMP_LIBDIR}"
		export LD_LIBRARY_PATH
	fi
}

############################# clear_freebl #############################
# local shell function to set default library path and clear temporary 
# directory for libraries created by function set_freebl 
########################################################################
clear_freebl()
{
	LD_LIBRARY_PATH="${OLD_LIBRARY_PATH}"
	export LD_LIBRARY_PATH

	if [ -d "${TMP_LIBDIR}" ] ; then
		rm -rf ${TMP_LIBDIR}
	fi
}

############################ run_command_dbx ###########################
# local shell function to run command under dbx tool
########################################################################
run_command_dbx()
{
	COMMAND=$1
	shift
	ATTR=$*

	COMMAND=`which ${COMMAND}`
	DBX_CMD="dbxenv follow_fork_mode parent
dbxenv rtc_mel_at_exit verbose
dbxenv rtc_biu_at_exit verbose
check -memuse -match 16 -frames 16
run ${ATTR}
"

	echo "${SCRIPTNAME}: -------- Running ${COMMAND} under DBX:"
	echo "${DBX} ${COMMAND}"
	echo "${SCRIPTNAME}: -------- DBX commands:"
	echo "${DBX_CMD}"
	echo "${DBX_CMD}" | ${DBX} ${COMMAND} 2>/dev/null | grep -v Reading
}

######################### run_command_valgrind #########################
# local shell function to run command under valgrind tool
########################################################################
run_command_valgrind()
{
	COMMAND=$1
	shift
	ATTR=$*

	echo "${SCRIPTNAME}: -------- Running ${COMMAND} under Valgrind:"
	echo "${VALGRIND} --tool=memcheck --leak-check=yes --show-reachable=yes --partial-loads-ok=yes --leak-resolution=high --num-callers=50 ${COMMAND} ${ATTR}"
	${VALGRIND} --tool=memcheck --leak-check=yes --show-reachable=yes --partial-loads-ok=yes --leak-resolution=high --num-callers=50 ${COMMAND} ${ATTR} 
	echo "==0==" 
}

############################# run_selfserv #############################
# local shell function to start selfserv
########################################################################
run_selfserv()
{
	echo "PATH=${PATH}"
	echo "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
	echo "${SCRIPTNAME}: -------- Running selfserv:"
	echo "selfserv ${SELFSERV_ATTR}"
	selfserv ${SELFSERV_ATTR}
}

########################### run_selfserv_dbg ###########################
# local shell function to start selfserv under debug tool
########################################################################
run_selfserv_dbg()
{
	echo "PATH=${PATH}"
	echo "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
	${RUN_COMMAND_DBG} selfserv ${SERVER_OPTION} ${SELFSERV_ATTR}
}

############################# run_strsclnt #############################
# local shell function to run strsclnt for all ciphers and send stop
# command to selfserv over tstclnt
########################################################################
run_strsclnt()
{
	for cipher in ${cipher_list}; do
		STRSCLNT_ATTR="-q -p ${PORT} -d ${CLIENT_DB} -w nss -c 1000 -C ${cipher} ${HOSTADDR}"
		echo "${SCRIPTNAME}: -------- Trying cipher ${cipher}:"
		echo "strsclnt ${STRSCLNT_ATTR}"
		strsclnt ${STRSCLNT_ATTR}
	done
	
	echo "${SCRIPTNAME}: -------- Stopping server:"
	echo "tstclnt ${TSTCLNT_ATTR} < ${REQUEST_FILE}"
	tstclnt ${TSTCLNT_ATTR} < ${REQUEST_FILE}
}

########################### run_strsclnt_dbg ###########################
# local shell function to run strsclnt under debug tool for all ciphers 
# and send stop command to selfserv over tstclnt
########################################################################
run_strsclnt_dbg()
{
	for cipher in ${cipher_list}; do
		STRSCLNT_ATTR="-q -p ${PORT} -d ${CLIENT_DB} -w nss -c 1001 -C ${cipher} ${HOSTADDR}"
		${RUN_COMMAND_DBG} strsclnt ${CLIENT_OPTION} ${STRSCLNT_ATTR}
	done
	
	echo "${SCRIPTNAME}: -------- Stopping server:"
	echo "tstclnt ${TSTCLNT_ATTR} < ${REQUEST_FILE}"
	tstclnt ${TSTCLNT_ATTR} < ${REQUEST_FILE}
}

########################## run_ciphers_server ##########################
# local shell function to test server part of code (selfserv)
########################################################################
run_ciphers_server()
{
	html_head "Memory leak checking - server"
	
	client_mode="NORMAL"	
	for server_mode in ${MODE_LIST}; do
		set_test_mode
		
		for freebl in ${FREEBL_LIST}; do
			set_freebl
			
			LOGNAME=server-${BIT_NAME}-${freebl}-${server_mode}
			LOGFILE=${LOGDIR}/${LOGNAME}.log
			echo "Running ${LOGNAME}"
			
			run_selfserv_dbg 2>&1 | tee ${LOGFILE} &
			sleep 5
			run_strsclnt
			
			sleep 20
			clear_freebl
			
			log_parse >> ${FOUNDLEAKS}
			ret=$?
			html_msg ${ret} 0 "${LOGNAME}" "produced a returncode of $ret, expected is 0"
		done
	done
	
	html "</TABLE><BR>"
}

########################## run_ciphers_client ##########################
# local shell function to test client part of code (strsclnt)
########################################################################
run_ciphers_client()
{
	html_head "Memory leak checking - client"
	
	server_mode="NORMAL"
	for client_mode in ${MODE_LIST}; do
		set_test_mode
		
		for freebl in ${FREEBL_LIST}; do
			set_freebl
			
			LOGNAME=client-${BIT_NAME}-${freebl}-${client_mode}
			LOGFILE=${LOGDIR}/${LOGNAME}.log
			echo "Running ${LOGNAME}"
			
			run_selfserv &
			sleep 5
			run_strsclnt_dbg 2>&1 | tee ${LOGFILE}
			
			sleep 20
			clear_freebl
			
			log_parse >> ${FOUNDLEAKS}
			ret=$?
			html_msg ${ret} 0 "${LOGNAME}" "produced a returncode of $ret, expected is 0"
		done
	done
	
	html "</TABLE><BR>"
}

########################## parse_logfile_dbx ###########################
# local shell function to parse and process logs from dbx
########################################################################
parse_logfile_dbx()
{
	in_mel=0
	bin_name=""
	
	while read line
	do
		if [ "${line}" = "Memory Leak (mel):" -o "${line}" = "Possible memory leak -- address in block (aib):" \
			-o "${line}" = "Block in use (biu):" ] ; then
			in_mel=1
			mel_line=0
			stack_string=""
		fi
		
		if [ -z "${line}" ] ; then
			if [ ${in_mel} -eq "1" ] ; then
				in_mel=0
				echo "${bin_name}${stack_string}"
			fi
		fi
			
		if [ ${in_mel} -eq 1 ] ; then
			mel_line=`expr ${mel_line} + 1`
			
			if [ ${mel_line} -ge 4 ] ; then
				new_line=`echo "${line}" | cut -d" " -f2 | cut -d"(" -f1`
				stack_string="/${new_line}${stack_string}"
			fi
				
			gline=`echo "${line}" | grep "Found leaked block of size"`
			if [ -n "${gline}" ] ; then
				lbytes=`echo "${line}" | sed "s/Found leaked block of size \(.*\) bytes.*/\1/"`
				tbytes=`expr "${tbytes}" + "${lbytes}"`
				tblocks=`expr "${tblocks}" + 1`
			fi
		else
			gline=`echo "${line}" | grep "^Running: "`
			if [ -n "${gline}" ] ; then
				bin_name=`echo "${line}" | cut -d" " -f2`
			fi
		fi 
	done
}

######################## parse_logfile_valgrind ########################
# local shell function to parse and process logs from valgrind
########################################################################
parse_logfile_valgrind()
{
	in_mel=0
	in_sum=0
	bin_name=""

	while read line
	do
		gline=`echo "${line}" | grep "^=="`
		if [ -z "${gline}" ] ; then
			gline=`echo "${line}" | grep "^${VALGRIND} " | sed "s:.*/::"`
			if [ -n "${gline}" ] ; then
				bin_name=`echo "${line}" | cut -d" " -f8`
			fi
			continue
		fi

		line=`echo "${line}" | sed "s/==[0-9]*==\s*\(.*\)/\1/"`

		gline=`echo "${line}" | grep "blocks are"`
		if [ -n "${gline}" ] ; then
			in_mel=1
			mel_line=0
			stack_string=""
		else
			gline=`echo "${line}" | grep "LEAK SUMMARY"`
			if [ -n "${gline}" ] ; then
				in_sum=1
				mel_line=0
			fi
		fi
		
		if [ -z "${line}" ] ; then
			if [ ${in_mel} -eq 1 ] ; then
				in_mel=0
				echo "${bin_name}${stack_string}"
			elif [ ${in_sum} -eq 1 ] ; then
				in_sum=0
			fi
		fi
			
		if [ ${in_mel} -eq 1 ] ; then
			mel_line=`expr ${mel_line} + 1`
			
			if [ ${mel_line} -ge 2 ] ; then
				new_line=`echo "${line}" | sed "s/[^:]*:\ \(\S*\).*/\1/"`
				if [ "${new_line}" = "(within" ] ; then
					new_line="*"
				fi
				stack_string="/${new_line}${stack_string}"
			fi
		elif [ ${in_sum} -eq 1 ] ; then
			mel_line=`expr ${mel_line} + 1`
			
			if [ ${mel_line} -ge 2 ] ; then
				gline=`echo "${line}" | grep "bytes.*blocks"`
				if [ -n "${gline}" ] ; then
					lbytes=`echo "${line}" | sed "s/.*: \(.*\) bytes.*/\1/" | sed "s/,//g"`
					lblocks=`echo "${line}" | sed "s/.*bytes in \(.*\) blocks.*/\1/" | sed "s/,//g"`

					tbytes=`expr "${tbytes}" + "${lbytes}"`
					tblocks=`expr "${tblocks}" + "${lblocks}"`
				else
					in_sum=0
				fi
			fi
		fi
	done
}

############################# log_compare ##############################
# local shell function to check if selected stack is found in the list
# of ignored stacks
########################################################################
log_compare()
{
	BUG_ID=""

	while read line
	do
		LINE="${line}"
		STACK="${stack}"

		if [ "${LINE}" = "${STACK}" ] ; then
			return 0
		fi
		
		NEXT=0

		gline=`echo "${LINE}" | grep '^#'`
		if [ -n "${gline}" ] ; then
			BUG_ID="${LINE}"
			NEXT=1
		fi
		
		gline=`echo "${LINE}" | grep '*'`
		if [ -z "${gline}" ] ; then
			NEXT=1
		fi
		
		while [ "${LINE}" != "" -a ${NEXT} -ne 1 ]
		do
			L_WORD=`echo "${LINE}" | cut -d '/' -f1`
			gline=`echo "${LINE}" | grep '/'`
			if [ -n "${gline}" ] ; then
				LINE=`echo "${LINE}" | cut -d '/' -f2-`
			else
				LINE=""
			fi

			S_WORD=`echo "${STACK}" | cut -d '/' -f1`
			gline=`echo "${STACK}" | grep '/'`
			if [ -n "${gline}" ] ; then
				STACK=`echo "${STACK}" | cut -d '/' -f2-`
			else
				STACK=""
			fi
			
			if [ "${L_WORD}" = "**" ] ; then
				return 0
			fi
			
			if [ "${L_WORD}" != "${S_WORD}" -a "${L_WORD}" != "*" -a "${S_WORD}" != "*" ] ; then
				NEXT=1
				break
			fi
		done

		if [ "${LINE}" = "" -a "${STACK}" = "" ] ; then
			return 0
		fi
	done

	return 1
}

############################# check_ignored ############################
# local shell function to check all stacks if they are not ignored
########################################################################
check_ignored()
{
	ret=0

	while read stack
	do
		log_compare < ${IGNORED_STACKS}
		if [ $? -eq 0 ] ; then
			if [ ${BUG_ID} != "" ] ; then
				echo "IGNORED STACK (${BUG_ID}): ${stack}"
			else
				echo "IGNORED STACK: ${stack}"
			fi
		else
			ret=1
			echo "NEW STACK: ${stack}"
		fi
	done

	return ${ret}
}

############################### parse_log ##############################
# local shell function to parse log file
########################################################################
log_parse()
{
	echo "${SCRIPTNAME}: Processing log ${LOGNAME}:"
	${PARSE_LOGFILE} < ${LOGFILE} > ${TMP_STACKS}
	cat ${TMP_STACKS} | sort -u | check_ignored
	ret=$?
	rm ${TMP_STACKS}
	echo ""

	return ${ret}
}

############################## cnt_total ###############################
# local shell function to count total leaked bytes
########################################################################
cnt_total()
{
	echo ""
	echo "TinderboxPrint:${OPT} Lk bytes: ${tbytes}"
	echo "TinderboxPrint:${OPT} Lk blocks: ${tblocks}"
	echo ""
}

############################### run_ocsp ###############################
# local shell function to run ocsp tests
########################################################################
run_ocsp()
{
	cd ${QADIR}/iopr
	. ./ocsp_iopr.sh
	ocsp_iopr_run
}

################################# main #################################

memleak_init

run_ciphers_server
run_ciphers_client
run_ocsp

cnt_total
memleak_cleanup

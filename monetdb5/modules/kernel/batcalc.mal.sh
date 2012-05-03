# The contents of this file are subject to the MonetDB Public License
# Version 1.1 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.monetdb.org/Legal/MonetDBLicense
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
#
# The Original Code is the MonetDB Database System.
#
# The Initial Developer of the Original Code is CWI.
# Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
# Copyright August 2008-2012 MonetDB B.V.
# All Rights Reserved.

cat <<EOF
# The contents of this file are subject to the MonetDB Public License
# Version 1.1 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.monetdb.org/Legal/MonetDBLicense
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
#
# The Original Code is the MonetDB Database System.
#
# The Initial Developer of the Original Code is CWI.
# Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
# Copyright August 2008-2012 MonetDB B.V.
# All Rights Reserved.

# This file was generated by using the script $0.

module batcalc;

EOF

integer="bte sht int wrd lng"	# all integer types
numeric="$integer flt dbl"	# all numeric types
alltypes="bit $numeric oid str"

for tp in $numeric; do
    cat <<EOF
command iszero(b:bat[:oid,:$tp]) :bat[:oid,:bit]
address CMDbatISZERO
comment "Unary check for zero over the tail of the bat";

EOF
done
echo

for tp in $alltypes; do
    cat <<EOF
command isnil(b:bat[:oid,:$tp]) :bat[:oid,:bit]
address CMDbatISNIL
comment "Unary check for nil over the tail of the bat";

EOF
done
echo

com="Return the Boolean inverse"
for tp in bit $integer; do
    cat <<EOF
command not(b:bat[:oid,:$tp]) :bat[:oid,:$tp]
address CMDbatNOT
comment "$com";
command not(b:bat[:oid,:$tp],accum:int) :bat[:oid,:$tp]
address CMDbatNOTaccum
comment "$com, reuse B if ACCUM is set";

EOF
    com="Unary bitwise not over the tail of the bat"
done
echo

for tp in $numeric; do
    cat <<EOF
command sign(b:bat[:oid,:$tp]) :bat[:oid,:bte]
address CMDbatSIGN
comment "Unary sign (-1,0,1) over the tail of the bat";

EOF
done
echo

for func in 'abs:ABS:Unary abs over the tail of the bat' \
    '-:NEG:Unary neg over the tail of the bat' \
    '++:INCR:Unary increment over the tail of the bat' \
    '--:DECR:Unary decrement over the tail of the bat'; do
    op=${func%%:*}
    com=${func##*:}
    func=${func%:*}
    func=${func#*:}
    for tp in $numeric; do
	cat <<EOF
command $op(b:bat[:oid,:$tp]) :bat[:oid,:$tp]
address CMDbat${func}
comment "$com";
command $op(b:bat[:oid,:$tp],accum:int) :bat[:oid,:$tp]
address CMDbat${func}accum
comment "$com, reuse B if ACCUM is set";

EOF
    done
    echo
done

for func in +:ADD -:SUB \*:MUL; do
    name=${func#*:}
    op=${func%:*}
    for tp1 in bte sht int lng flt; do
	for tp2 in bte sht int lng flt; do
	    case $tp1$tp2 in
	    *flt*) tp3=dbl;;
	    *lng*) continue;;	# lng only allowed in combination with flt
	    *int*) tp3=lng;;
	    *sht*) tp3=int;;
	    *bte*) tp3=sht;;
	    esac
	    cat <<EOF
command $op(b1:bat[:oid,:$tp1],b2:bat[:oid,:$tp2]) :bat[:oid,:$tp3]
address CMDbat${name}enlarge
comment "Return B1 $op B2, guarantee no overflow by returning larger type";
pattern $op(b:bat[:oid,:$tp1],v:$tp2) :bat[:oid,:$tp3]
address CMDbat${name}cstenlarge
comment "Return B $op V, guarantee no overflow by returning larger type";
pattern $op(v:$tp1,b:bat[:oid,:$tp2]) :bat[:oid,:$tp3]
address CMDbat${name}cstsignal
comment "Return V $op B, guarantee no overflow by returning larger type";

EOF
	done
    done
    echo
done

for func in +:ADD -:SUB \*:MUL; do
    name=${func#*:}
    op=${func%:*}
    for tp1 in $numeric; do
	for tp2 in $numeric; do
	    case $tp1$tp2 in
	    *dbl*) tp3=dbl;;
	    *flt*) tp3=flt;;
	    *lng*) tp3=lng;;
	    *wrd*) tp3=wrd;;
	    *int*) tp3=int;;
	    *sht*) tp3=sht;;
	    *bte*) tp3=bte;;
	    esac
	    cat <<EOF
command $op(b1:bat[:oid,:$tp1],b2:bat[:oid,:$tp2]) :bat[:oid,:$tp3]
address CMDbat${name}signal
comment "Return B1 $op B2, signal error on overflow";
command $op(b1:bat[:oid,:$tp1],b2:bat[:oid,:$tp2],accum:int) :bat[:oid,:$tp3]
address CMDbat${name}signalaccum
comment "Return B1 $op B2, signal error on overflow, reuse B1 or B2 depending on ACCUM";
command ${name,,}_noerror(b1:bat[:oid,:$tp1],b2:bat[:oid,:$tp2]) :bat[:oid,:$tp3]
address CMDbat${name}
comment "Return B1 $op B2, overflow causes NIL value";
pattern $op(b:bat[:oid,:$tp1],v:$tp2) :bat[:oid,:$tp3]
address CMDbat${name}cstsignal
comment "Return B $op V, signal error on overflow";
EOF
	    if [ $tp1 = $tp3 ]; then
cat <<EOF
pattern $op(b:bat[:oid,:$tp1],v:$tp2,accum:int) :bat[:oid,:$tp3]
address CMDbat${name}cstsignal
comment "Return B $op V, signal error on overflow, reuse B if ACCUM is set";
EOF
	    fi
cat <<EOF
pattern ${name,,}_noerror(b:bat[:oid,:$tp1],v:$tp2) :bat[:oid,:$tp3]
address CMDbat${name}cst
comment "Return B $op V, overflow causes NIL value";
pattern $op(v:$tp1,b:bat[:oid,:$tp2]) :bat[:oid,:$tp3]
address CMDbat${name}cstsignal
comment "Return V $op B, signal error on overflow";
EOF
	    if [ $tp2 = $tp3 ]; then
cat <<EOF
pattern $op(v:$tp1,b:bat[:oid,:$tp2],accum:int) :bat[:oid,:$tp3]
address CMDbat${name}cstsignal
comment "Return V $op B, signal error on overflow, reuse B if ACCUM is set";
EOF
	    fi
cat <<EOF
pattern ${name,,}_noerror(v:$tp1,b:bat[:oid,:$tp2]) :bat[:oid,:$tp3]
address CMDbat${name}cst
comment "Return V $op B, overflow causes NIL value";

EOF
	done
    done
    echo
done

for tp1 in $numeric; do
    for tp2 in $numeric; do
	case $tp1$tp2 in
	*dbl*) tp3=dbl;;
	*flt*) tp3=flt;;
	lng*) tp3=lng;;
	wrd*) tp3=wrd;;
	int*) tp3=int;;
	sht*) tp3=sht;;
	bte*) tp3=bte;;
	esac
	cat <<EOF
command /(b1:bat[:oid,:$tp1],b2:bat[:oid,:$tp2]) :bat[:oid,:$tp3]
address CMDbatDIVsignal
comment "Return B1 / B2, signal error on overflow";
command /(b1:bat[:oid,:$tp1],b2:bat[:oid,:$tp2],accum:int) :bat[:oid,:$tp3]
address CMDbatDIVsignalaccum
comment "Return B1 / B2, signal error on overflow, reuse B1 or B2 depending on ACCUM";
command div_noerror(b1:bat[:oid,:$tp1],b2:bat[:oid,:$tp2]) :bat[:oid,:$tp3]
address CMDbatDIV
comment "Return B1 / B2, overflow causes NIL value";
pattern /(b:bat[:oid,:$tp1],v:$tp2) :bat[:oid,:$tp3]
address CMDbatDIVcstsignal
comment "Return B / V, signal error on overflow";
EOF
	    if [ $tp1 = $tp3 ]; then
cat <<EOF
pattern /(b:bat[:oid,:$tp1],v:$tp2,accum:int) :bat[:oid,:$tp3]
address CMDbatDIVcstsignal
comment "Return B / V, signal error on overflow, reuse B if ACCUM is set";
EOF
	    fi
	    cat <<EOF
pattern div_noerror(b:bat[:oid,:$tp1],v:$tp2) :bat[:oid,:$tp3]
address CMDbatDIVcst
comment "Return B / V, overflow causes NIL value";
pattern /(v:$tp1,b:bat[:oid,:$tp2]) :bat[:oid,:$tp3]
address CMDbatDIVcstsignal
comment "Return V / B, signal error on overflow";
EOF
	    if [ $tp2 = $tp3 ]; then
cat <<EOF
pattern /(v:$tp1,b:bat[:oid,:$tp2],accum:int) :bat[:oid,:$tp3]
address CMDbatDIVcstsignal
comment "Return V / B, signal error on overflow, reuse B if ACCUM is set";
EOF
	    fi
	    cat <<EOF
pattern div_noerror(v:$tp1,b:bat[:oid,:$tp2]) :bat[:oid,:$tp3]
address CMDbatDIVcst
comment "Return V / B, overflow causes NIL value";

EOF
    done
done
    echo

for tp1 in $integer; do
    for tp2 in $integer; do
	case $tp1$tp2 in
	*bte*) tp3=bte;;
	*sht*) tp3=sht;;
	*int*) tp3=int;;
	*wrd*) tp3=wrd;;
	*lng*) tp3=lng;;
	esac
	cat <<EOF
command %(b1:bat[:oid,:$tp1],b2:bat[:oid,:$tp2]) :bat[:oid,:$tp3]
address CMDbatMODsignal
comment "Return B1 % B2, signal error on divide by zero";
command %(b1:bat[:oid,:$tp1],b2:bat[:oid,:$tp2],accum:int) :bat[:oid,:$tp3]
address CMDbatMODsignalaccum
comment "Return B1 % B2, signal error on divide by zero, reuse B1 or B2 depending on ACCUM";
command mod_noerror(b1:bat[:oid,:$tp1],b2:bat[:oid,:$tp2]) :bat[:oid,:$tp3]
address CMDbatMOD
comment "Return B1 % B2, divide by zero causes NIL value";
pattern %(b:bat[:oid,:$tp1],v:$tp2) :bat[:oid,:$tp3]
address CMDbatMODcstsignal
comment "Return B % V, signal error on divide by zero";
EOF
	if [ $tp1 = $tp3 ]; then
	    cat <<EOF
pattern %(b:bat[:oid,:$tp1],v:$tp2,accum:int) :bat[:oid,:$tp3]
address CMDbatMODcstsignal
comment "Return B % V, signal error on divide by zero, reuse B if ACCUM is set";
EOF
	fi
	cat <<EOF
pattern mod_noerror(b:bat[:oid,:$tp1],v:$tp2) :bat[:oid,:$tp3]
address CMDbatMODcst
comment "Return B % V, divide by zero causes NIL value";
pattern %(v:$tp1,b:bat[:oid,:$tp2]) :bat[:oid,:$tp3]
address CMDbatMODcstsignal
comment "Return V % B, signal error on divide by zero";
EOF
	if [ $tp2 = $tp3 ]; then
	    cat <<EOF
pattern %(v:$tp1,b:bat[:oid,:$tp2],accum:int) :bat[:oid,:$tp3]
address CMDbatMODcstsignal
comment "Return V % B, signal error on divide by zero, reuse B if ACCUM is set";
EOF
	fi
	cat <<EOF
pattern mod_noerror(v:$tp1,b:bat[:oid,:$tp2]) :bat[:oid,:$tp3]
address CMDbatMODcst
comment "Return V % B, divide by zero causes NIL value";

EOF
    done
done
echo

for op in and or xor; do
    for tp in bit $integer; do
	cat <<EOF
command ${op}(b1:bat[:oid,:$tp],b2:bat[:oid,:$tp]) :bat[:oid,:$tp]
address CMDbat${op^^}
comment "Return B1 ${op^^} B2";
command $op(b1:bat[:oid,:$tp],b2:bat[:oid,:$tp], accum:int) :bat[:oid,:$tp]
address CMDbat${op^^}accum
comment "Return B1 ${op^^} B2";
pattern $op(b:bat[:oid,:$tp],v:$tp) :bat[:oid,:$tp]
address CMDbat${op^^}cst
comment "Return B ${op^^} V";
pattern $op(b:bat[:oid,:$tp],v:$tp,accum:int) :bat[:oid,:$tp]
address CMDbat${op^^}cst
comment "Return B ${op^^} V";
pattern $op(v:$tp,b:bat[:oid,:$tp]) :bat[:oid,:$tp]
address CMDbat${op^^}cst
comment "Return V ${op^^} B";
pattern $op(v:$tp,b:bat[:oid,:$tp],accum:int) :bat[:oid,:$tp]
address CMDbat${op^^}cst
comment "Return V ${op^^} B";

EOF
    done
    echo
done

for func in '<<:lsh' '>>:rsh'; do
    op=${func%:*}
    func=${func#*:}
    for tp1 in $integer; do
	for tp2 in $integer; do
	    cat <<EOF
command $op(b1:bat[:oid,:$tp1],b2:bat[:oid,:$tp2]) :bat[:oid,:$tp1]
address CMDbat${func^^}signal
comment "Return B1 $op B2, raise error on out of range second operand";
command $op(b1:bat[:oid,:$tp1],b2:bat[:oid,:$tp2],accum:int) :bat[:oid,:$tp1]
address CMDbat${func^^}signalaccum
comment "Return B1 $op B2, raise error on out of range second operand, reuse B if ACCUM is set";
command ${func}_noerror(b1:bat[:oid,:$tp1],b2:bat[:oid,:$tp2]) :bat[:oid,:$tp1]
address CMDbat${func^^}
comment "Return B1 $op B2, out of range second operand causes NIL value";
pattern $op(b:bat[:oid,:$tp1],v:$tp2) :bat[:oid,:$tp1]
address CMDbat${func^^}cstsignal
comment "Return B $op V, raise error on out of range second operand";
pattern $op(b:bat[:oid,:$tp1],v:$tp2,accum:int) :bat[:oid,:$tp1]
address CMDbat${func^^}cstsignal
comment "Return B $op V, raise error on out of range second operand, reuse B if ACCUM is set";
pattern ${func}_noerror(b:bat[:oid,:$tp1],v:$tp2) :bat[:oid,:$tp1]
address CMDbat${func^^}cst
comment "Return B $op V, out of range second operand causes NIL value";
pattern $op(v:$tp1,b:bat[:oid,:$tp2]) :bat[:oid,:$tp1]
address CMDbat${func^^}cstsignal
comment "Return V $op B, raise error on out of range second operand";
pattern ${func}_noerror(v:$tp1,b:bat[:oid,:$tp2]) :bat[:oid,:$tp1]
address CMDbat${func^^}cst
comment "Return V $op B, out of range second operand causes NIL value";

EOF
	done
    done
    echo
done

for func in '<:lt' '<=:le' '>:gt' '>=:ge' '==:eq' '!=:ne'; do
    op=${func%:*}
    func=${func#*:}
    for tp in bit str oid; do
	cat <<EOF
command $op(b1:bat[:oid,:$tp],b2:bat[:oid,:$tp]) :bat[:oid,:bit]
address CMDbat${func^^}
comment "Return B1 $op B2";
pattern $op(b:bat[:oid,:$tp],v:$tp) :bat[:oid,:bit]
address CMDbat${func^^}cst
comment "Return B $op V";
pattern $op(v:$tp,b:bat[:oid,:$tp]) :bat[:oid,:bit]
address CMDbat${func^^}cst
comment "Return V $op B";

EOF
    done
    for tp1 in $numeric; do
	for tp2 in $numeric; do
	    cat <<EOF
command $op(b1:bat[:oid,:$tp1],b2:bat[:oid,:$tp2]) :bat[:oid,:bit]
address CMDbat${func^^}
comment "Return B1 $op B2";
pattern $op(b:bat[:oid,:$tp1],v:$tp2) :bat[:oid,:bit]
address CMDbat${func^^}cst
comment "Return B $op V";
pattern $op(v:$tp1,b:bat[:oid,:$tp2]) :bat[:oid,:bit]
address CMDbat${func^^}cst
comment "Return V $op B";

EOF
	done
    done
    echo
done

op=${func%:*}
func=${func#*:}
for tp in bit str oid; do
    cat <<EOF
command cmp(b1:bat[:oid,:$tp],b2:bat[:oid,:$tp]) :bat[:oid,:bte]
address CMDbatCMP
comment "Return -1/0/1 if B1 </==/> B2";
pattern cmp(b:bat[:oid,:$tp],v:$tp) :bat[:oid,:bte]
address CMDbatCMPcst
comment "Return -1/0/1 if B </==/> V";
pattern cmp(v:$tp,b:bat[:oid,:$tp]) :bat[:oid,:bte]
address CMDbatCMPcst
comment "Return -1/0/1 if V </==/> B";

EOF
done
for tp1 in $numeric; do
    for tp2 in $numeric; do
	cat <<EOF
command cmp(b1:bat[:oid,:$tp1],b2:bat[:oid,:$tp2]) :bat[:oid,:bte]
address CMDbatCMP
comment "Return -1/0/1 if B1 </==/> B2";
pattern cmp(b:bat[:oid,:$tp1],v:$tp2) :bat[:oid,:bte]
address CMDbatCMPcst
comment "Return -1/0/1 if B </==/> V";
pattern cmp(v:$tp1,b:bat[:oid,:$tp2]) :bat[:oid,:bte]
address CMDbatCMPcst
comment "Return -1/0/1 if V </==/> B";

EOF
    done
done
echo

for tp in bit $numeric oid; do
    cat <<EOF
pattern between(b:bat[:oid,:$tp],lo:bat[:oid,:$tp],hi:bat[:oid,:$tp]) :bit
address CMDbatBETWEEN
comment "B between LO and HI inclusive, nil border is (minus) infinity";
pattern between(b:bat[:oid,:$tp],lo:bat[:oid,:$tp],hi:$tp) :bit
address CMDbatBETWEEN
comment "B between LO and HI inclusive, nil border is (minus) infinity";
pattern between(b:bat[:oid,:$tp],lo:$tp,hi:bat[:oid,:$tp]) :bit
address CMDbatBETWEEN
comment "B between LO and HI inclusive, nil border is (minus) infinity";
pattern between(b:bat[:oid,:$tp],lo:$tp,hi:$tp) :bit
address CMDbatBETWEEN
comment "B between LO and HI inclusive, nil border is (minus) infinity";

EOF
done
echo

for tp in $numeric; do
    cat <<EOF
command avg(b:bat[:oid,:$tp]) :dbl
address CMDcalcavg
comment "average of non-nil values of B";
command avg2(b:bat[:oid,:$tp]) (:dbl, :lng)
address CMDcalcavg2
comment "average and number of non-nil values of B";

EOF
done

for tp1 in $alltypes; do
    for tp2 in $alltypes; do
	cat <<EOF
command $tp1(b:bat[:any_1,:$tp2]) :bat[:any_1,:$tp1]
address CMDconvertsignal_$tp1
comment "cast from $tp2 to $tp1, signal error on overflow";
pattern $tp1(v:$tp2) :$tp1
address CMDconvertsignal
comment "cast from $tp2 to $tp1, signal error on overflow";
command ${tp1}_noerror(b:bat[:any_1,:$tp2]) :bat[:any_1,:$tp1]
address CMDconvert_$tp1
comment "cast from $tp2 to $tp1";
pattern ${tp1}_noerror(v:$tp2) :$tp1
address CMDconvert
comment "cast from $tp2 to $tp1";

EOF
    done
done

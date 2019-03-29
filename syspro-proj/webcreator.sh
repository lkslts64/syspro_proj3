#!/bin/bash


if [ ! -d "$1" ]; then
	echo "#Directory not exists .Exiting..."
	exit -1
fi
if [ ! -f $2 ]; then
    echo "#File not found.Exiting"
    exit -1
fi
re='^[0-9]+$'
if ! [[ $3 =~ $re ]] ; then
   echo "#Not a number" >&2; exit 1
fi
if ! [[ $4 =~ $re ]] ; then
   echo " #Not a number" >&2; exit 1
fi
filelength=$(cat $2 | wc -l)
if [[  filelength -lt 10000 ]] ; then
	echo "#Small file.Exiting";exit 1
fi
if [ "$(ls -A $1)" ]; then
     echo "#Warning : Purging $1..."
     rm -r "$1"*
fi


for (( c=0; c< $3 ; c++ ));do
	d="$1site$c"
	mkdir $d
	count=1
	while [  "${#entries[@]}" -lt "$4" ];do
		while [ "$count" -le $4 ]; do
		 number[$count]=$RANDOM
		 let "count += 1"
		done
		IFS=$'\n'
		entries=($(sort -u <<<"${number[*]}"))
		unset IFS
	done
	entries=("${entries[@]/#/"$d/page"$c"_"}")
	entries=("${entries[@]/%/".html"}")
	all=("${all[@]}" "${entries[@]}")
	unset entries



#for i in ${entries[@]}; do

#page="$d/page$c__nfdsfsd"
#entries=("${entries[@]/#/"$d/page$c/_omg"}")
#all=("${all[@]}" "${entries[@]}")

done

f=$(($4 / 2))
q=$(($3 / 2))
let "q+=1"
let "f+=1"

count4=0
count2=0
for (( c=0; c< $3 ; c++ ));do
 echo "#Creating website $c.."
 for (( v=0; v< $4 ; v++ ));do 
	d="$1site$c"
	RANGE1=$filelength-2000
	RANGE2=1000
	k=$RANDOM
	let "k %= $RANGE1"
	let "k+=2"
	m=$RANDOM
	let "m %= $RANGE2"
	let "m+=1001"
	
	count=1
	
	
	while [ "$count" -le $f ]; do					#for internal links..
		internal=${all[$RANDOM % ${#all[@]} ]}
		if echo "$internal" | grep -q "$d"; then
			if [ "${all[$count2]}" != "$internal" ]; then
				internals[$count]=$internal
				numofel1=${#internals[@]}
				IFS=$'\n'
				internals=($(sort -u <<<"${internals[*]}"))
				unset IFS
				if [ "$numofel1" -eq "${#internals[@]}" ]; then
					let "count += 1"
				fi
			fi
			
		fi
		
	done
	count=1
	while [ "$count" -le $q ]; do					#for external..
		external=${all[$RANDOM % ${#all[@]} ]}
		if echo "$external" | grep -qv "$d"; then
			externals[$count]=$external
			numofel1=${#externals[@]}
			IFS=$'\n'
			externals=($(sort -u <<<"${externals[*]}"))
			unset IFS
			
			if [ "$numofel1" -eq "${#externals[@]}" ]; then
				let "count += 1"
			fi
		fi
		
	done
	
	
	
	
	links=("${internals[@]}" "${externals[@]}")
	IFS=$'\n' 
	links=($(sort -R <<<"${links[*]}"))			#sort all links randomly . just because the instructor said...
	unset IFS
	
	touch "${all[$count2]}"
	echo -e "<!DOCTYPE html>\n<html>\n\t<body>\n" >> "${all[$count2]}"
	
	echo "	#Creating page ${all[$count2]} with $m lines starting at line $k ..."
	
	sum=$((f + q))		#f+q
	z=$((m / sum))		#m/(f+q)
	sum3=$z				#m/(f+q)
	sum2=$((z + k))		#m/(f+q)+k
	count3=0
	while [[ "$sum3" -lt "$m" || "$count3" -lt "${#links[@]}" ]] ; do
		#if [ "$sum3" -lt "$m" ]; then
			head -n $sum2 $2 | tail -$z >> "${all[$count2]}"
		#fi
		echo "		#Adding link to ${links[$count3]}"
		link=${links[$count3]#$1}
		link="/$link"
		incoming[$count4]=$link
		ran=$RANDOM
		echo -e " <a href=$link>link$ran</a>" >> "${all[$count2]}"
		let "sum3+=$z"						#sum3 is counter to exit while loop
		let "sum2+=$z"	
		let "count3+=1"	
		let "count4+=1"	
	done	
	
   let "count2 += 1"
   unset internals
   unset externals
   unset links
 done
done


mul=$(($4 * $3))
IFS=$'\n'
incoming=($(sort -u <<<"${incoming[*]}"))
unset IFS
if [ "${#incoming[@]}" -eq "$mul" ]; then
	echo "#All pages have at least one incoming link"
fi

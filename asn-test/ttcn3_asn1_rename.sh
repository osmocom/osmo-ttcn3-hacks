for f in *.asn; do nf=`echo $f | sed -e 's/-/_/'g`; mv $f $nf; done

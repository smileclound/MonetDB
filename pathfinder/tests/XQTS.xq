(
<h>
#!/bin/sh

if [ ! "$3" ] ; then
	echo 'Usage: '"$0"' {text{"<TSTSRCBASE> <XQTS_DIR> <XQTS_SRC>"}}'
	exit 1
fi
	
TSTSRCBASE="$1"
XQTS_DIR="$2"
XQTS_SRC="$3"
XQTS_DST="$TSTSRCBASE/$XQTS_DIR"
DOCS_DIR="$XQTS_DIR/doc"
DOCS_DST="$XQTS_DST/doc"

mkdir -p "$DOCS_DST"
cp "$XQTS_SRC/TestSources"/*.xml "$DOCS_DST"
</h>
,
for $tst in doc("XQTSCatalog.xml")//*:test-case
return
if ($tst/*:output-file or $tst/*:expected-error) then
<t>

TSTDIR="{fn:data($tst/@FilePath)}"
TSTDIR="${text{"{"}}TSTDIR%/{text{"}"}}"
TSTNME="{fn:data($tst/@name)}"
TSTFLE="{fn:data($tst/*:query/@name)}.xq"
mkdir -p "$XQTS_DST/$TSTDIR/Tests"
echo "$TSTNME" >> "$XQTS_DST/$TSTDIR/Tests/All"
cat "$XQTS_SRC/Queries/XQuery/$TSTDIR/$TSTFLE" \
{
for $i in $tst/*:input-file
return 
<x>
  | perl -pe 's|^declare variable (\${fn:data($i/@variable)}) external;|let $1 := doc("\$TSTSRCBASE/'"$DOCS_DIR"'/{$i/text()}.xml") return|' \
</x>
}
  > "$XQTS_DST/$TSTDIR/Tests/$TSTNME.xq.in"
(
echo 'stdout of test '\'"$TSTNME"'` in directory '\'"$XQTS_DIR/$TSTDIR"'` itself:'
echo ''
echo 'Ready.'
echo ''
echo 'Over..'
echo ''
{
if ($tst/*:output-file) then
<o>
echo -e '{text{"<?xml"}} version="1.0" encoding="utf-8"{text{"?>"}}\n<XQueryResult>'
cat "$XQTS_SRC/ExpectedTestResults/$TSTDIR/{$tst/*:output-file[1]/text()}"
echo -e '\n</XQueryResult>'
</o>
else
<o/>
}
) > "$XQTS_DST/$TSTDIR/Tests/$TSTNME.stable.out"
(
echo 'stderr of test '\'"$TSTNME"'` in directory '\'"$XQTS_DIR/$TSTDIR"'` itself:'
echo ''
{
if ($tst/*:expected-error) then
<e>
echo '! expecting {fn:data($tst/@scenario)} "{$tst/*:expected-error/text()}" (cf., http://www.w3.org/TR/xquery/#ERR{$tst/*:expected-error/text()} ) !'
</e>
else
<e/>
}
) > "$XQTS_DST/$TSTDIR/Tests/$TSTNME.stable.err"
</t>
else
<t>
echo 'not(*:output-file) and not(*:expected-error): {fn:data($tst/@FilePath)}{fn:data($tst/@name)}"
</t>
)

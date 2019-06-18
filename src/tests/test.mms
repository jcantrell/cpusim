      LOC   Data_Segment
      OCTA  1F
a     GREG  @
ABCD  BYTE  "ab"
      LOC   #123456789
Main  JMP   1F
      LOC   @+#4000
2H    LDB   $3,ABCD+1
      BZ    $3,1F; TRAP
# 3 "foo.mms"
      LOC   2B-4*10
1H    JMP   2B
      BSPEC 5
      TETRA &a<<8
      WYDE  a-$0
      ESPEC
      LOC   ABCD+2
      BYTE  "cd",#98

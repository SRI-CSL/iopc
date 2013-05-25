op G108 : -> Gcode [ctor metadata "(\
  (KeggID G00108))"] .


http://www.genome.jp/dbget-bin/www_bget?-f+k+glycan+G00108
ENTRY       G00108                      Glycan
NODE        4
            1   Cer        13     0
            2   Glc         5     0
            3   Gal        -3     0
            4   Neu5Ac    -13     0
EDGE        3
            1     2:b1    1:1  
            2     3:b1    2:4  
            3     4:a2    3:3  
///

op G109 : -> Gcode [ctor metadata "(\
  (KeggID G00109))"] .
op G110 : -> Gcode [ctor metadata "(\
  (KeggID G00110))"] .






op Chondroitin : -> Gcode [ctor metadata "(\
 (KeggID C00401 G11334)\
  (category GAG)\
  (synonyms \"Chondroitin Sulfate\"\
            \"Chondroitin-D-glucuronate\"))"] .

op Dermatan  : -> Gcode  [ctor metadata "(\
  (KeggID C001490)\
  (category GAG)\
  (synonyms \"Dermatan L-iduronate\"))"] .

http://www.genome.jp/dbget-bin/www_bget?-f+k+glycan+G11334
ENTRY       G11334                      Glycan
NODE        4
            1   *          15     0
            2   GalNAc      5     0
            3   GlcA       -5     0
            4   *         -15     0
EDGE        3
            1     2:b1    1    
            2     3:b1    2:3  
            3     4       3:4  
BRACKET     1  -9.0   2.0  -9.0  -2.0
            1  10.0  -2.0  10.0   2.0
            1 n
///


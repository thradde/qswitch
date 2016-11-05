/* GIMP RGBA C-Source image dump (TrayIcon.c) */

static const struct {
  unsigned int 	 width;
  unsigned int 	 height;
  unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
  unsigned char	 pixel_data[32 * 32 * 4 + 1];
} app_icon = {
  32, 32, 4,
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\374\333\377\0\0\0\0\0""4v\266\27""1u\266h.s\264\2311u\265"
  "\2723w\266\3062v\266\275-s\264\2430u\265y5w\2674\0\0\0\0\0\0\0\0%n\260\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0""7z\270\0\0\0\0"
  "\0""2w\266X8z\270\330b\227\311\377\210\261\327\377\247\306\343\377\264\316"
  "\347\377\267\321\352\377\270\322\351\377\263\316\347\377\233\275\336\377"
  "u\244\317\377D\202\275\3670t\264\201\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0""7y\267>7z\270c\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0""5x\267(9{\271\337w\245\321\377\251\310\345"
  "\377\241\302\341\377\230\275\337\377\223\271\335\377\221\270\334\377\221"
  "\270\334\377\221\270\334\377\222\271\335\377\232\276\337\377\247\306\343"
  "\377\277\326\353\377\232\275\336\377F\204\275\3733v\266N\0\0\0\0\0\0\0\0"
  """3w\266:L\210\300\3774w\266\204\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0""1v\265dS\215\303\377\237\300\341\377\227"
  "\274\336\377\220\267\334\377\221\267\335\377\221\270\335\377\221\270\335"
  "\377\221\270\335\377\221\270\335\377\221\270\335\377\221\270\335\377\221"
  "\270\335\377\221\270\335\377\217\266\334\377\235\277\340\377\276\325\354"
  "\377p\240\316\377/t\265\2243w\2668V\217\304\377\254\311\344\377.s\264\204"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0""1u\265"
  "{c\230\312\377\232\275\337\377\215\265\333\377\215\264\333\377\215\264\333"
  "\377\215\264\333\377\215\264\333\377\215\264\333\377\215\264\333\377\215"
  "\264\333\377\215\264\333\377\215\264\333\377\215\264\333\377\215\264\333"
  "\377\215\264\333\377\215\265\333\377\214\264\333\377\257\313\346\377\214"
  "\263\330\377V\216\304\377\270\321\351\377\247\305\342\377/t\264\204\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0""3v\266Y^\224\307\377"
  "\221\267\335\377\211\261\331\377\211\262\331\377\211\262\331\377\211\262"
  "\331\377\211\262\331\377\211\262\331\377\211\262\331\377\211\262\331\377"
  "\211\262\331\377\211\262\331\377\211\262\331\377\211\262\331\377\211\262"
  "\331\377\211\262\331\377\211\262\331\377\211\262\331\377\207\260\331\377"
  "\246\305\343\377\271\321\351\377\210\261\332\377\250\306\343\377/t\264\204"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0""4w\267\0""6x\266\31I\206\277\377"
  "\214\264\333\377\205\257\330\377\205\256\331\377\205\256\331\377\205\256"
  "\331\377\205\256\331\377\205\256\331\377\205\256\331\377\204\256\330\377"
  "\205\256\330\377\207\260\331\377\205\256\331\377\203\255\330\377\205\256"
  "\330\377\205\257\331\377\205\256\331\377\205\256\331\377\205\256\331\377"
  "\205\256\331\377\204\256\330\377\207\260\331\377\202\255\327\377\246\304"
  "\342\377/t\264\204\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0""6x\267"
  "\311\177\253\326\377\201\254\327\377\201\254\327\377\201\254\327\377\201"
  "\254\327\377\201\254\327\377\201\254\327\377\200\253\327\377\213\262\332"
  "\377\233\275\340\377\221\266\333\377\211\260\330\377\224\270\334\377\246"
  "\304\343\377\231\274\337\377\203\255\330\377\201\253\327\377\201\254\327"
  "\377\201\254\327\377\201\254\327\377\201\254\327\377\201\254\327\377~\252"
  "\326\377\244\303\341\377/t\265\204\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0""2w\266"
  "\0""4w\267;X\220\306\377}\251\326\377}\251\325\377}\250\325\377}\250\325"
  "\377}\250\325\377}\251\325\377\177\251\326\377\220\266\334\377d\230\312\377"
  "9{\271\3340t\265\2100u\265o0u\265\2033v\266\305[\222\306\377\240\300\340"
  "\377\222\267\334\377{\247\325\377}\250\325\377}\250\325\377}\250\325\377"
  "}\250\325\377z\247\325\377\242\301\340\377/t\265\204\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0""4w\267\270w\245\324\377y\246\325\377y\246\325\377y"
  "\246\325\377y\246\325\377y\246\325\377z\246\325\377\205\256\330\376@\200"
  "\273\3653v\267?\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0""4x\266\31""4w\266"
  "\325\216\264\332\377\223\267\335\377y\245\324\377y\246\325\377y\246\325\377"
  "y\246\325\377v\243\323\377\240\300\340\3770t\265\204\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0H\205\277\377u\244\323\377t\242\322\377t\242\322\377"
  "t\242\322\377t\242\322\377t\242\322\377\201\253\327\377?~\273\3637z\267\13"
  ":}\270\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0""3w\266AO\211\301\377\236"
  "\277\340\377v\244\323\377t\242\322\377t\242\322\377t\242\322\377t\242\322"
  "\377q\240\322\377\236\277\337\3770t\265\204\0\0\0\0\0\0\0\0\0\0\0\0""2v\265"
  "\0""5w\267GX\217\307\377p\237\321\377p\240\322\377p\240\322\377p\240\322"
  "\377p\237\321\377u\243\323\377U\216\305\3774w\2675;}\271\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0""5x\267UU\216\304\377\241\301\341\377\217\264"
  "\333\377\217\264\333\377\220\265\334\377\222\266\334\377\223\267\335\377"
  "\223\267\335\377\222\265\334\377\234\275\336\3770t\265\204\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0""6y\267~P\212\304\377W\216\307\377W\216\307\377W\216\307"
  "\377W\216\307\377W\216\307\377^\223\311\3778z\270\317\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0""6x\267C8z\270\377R\213\303\377P\212\302\377"
  "Q\212\302\377R\213\302\377S\214\303\377S\214\303\377T\215\303\377U\215\304"
  "\377U\215\304\377P\212\302\3775x\267\200\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  """6y\267\35""5x\267G5w\267F5w\267F5w\267F5w\267F5w\267F4w\267F7y\270\40\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0""2u\265\0\0\0\0\0\0\0\0\0""8p\21\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0Ey\15\0@u\14\16Bw\13\"Bw\14\"Bw\14"
  "\"Bw\14\"Bw\14\"Bw\14\"Bw\14#>u\14\12$a\40\0\0\0\0\0\0\0\0\0\0\0\0\0?u\13"
  "wR\203\34\354T\204\36\354T\204\36\354T\204\36\354S\204\35\354S\203\34\354"
  "S\203\33\354R\203\32\354R\202\31\354U\204\34\353Av\14\367?u\13:\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0Bw\15\322q\231'\377l\226\40\377l\226"
  "\40\377l\226\40\377l\226\40\377l\226\40\377a\216\33\377?t\14}\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0""6o\2\212\275\320\201\377\276\321s\377\277\322u\377"
  "\277\322u\377\276\321s\377\275\320p\377\273\317l\377\272\316k\377\306\326"
  "~\377r\2323\377>t\11l\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0Au\13\0>t\13"
  "-k\225%\377\244\276A\377\240\273:\377\240\273:\377\240\273:\377\240\273:"
  "\377\240\273:\377w\235&\377>t\12?\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0""6o\2\212"
  "\275\320\203\377\243\2768\377\245\300=\377\245\300=\377\245\300=\377\245"
  "\300>\377\246\301?\377\302\324x\377f\222+\377;r\7T\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\300\377\"\0Ar\26\4H{\20\354\256\306P\377\246\300>\377"
  "\246\301?\377\246\301?\377\246\301?\377\246\301?\377\251\302@\377[\212\31"
  "\377\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0""6n\2\212\300\323\205\377\252"
  "\304?\377\254\306D\377\254\306D\377\254\306D\377\254\305C\377\277\323n\377"
  "\244\300^\3779p\4\307\1\0\2\5\0\0\0\5\1\2\0\12\0\0\0\13\0\0\0\13\0\0\0\4"
  """8g\12.H|\20\353\254\305T\377\256\306G\377\255\306D\377\254\306D\377\254"
  "\306D\377\254\306D\377\254\306D\377\253\304C\377<s\11\267\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0""5n\2\212\304\326\207\377\261\311E\377\262\312I"
  "\377\262\312I\377\262\312I\377\262\312I\377\262\312G\377\305\327r\377\264"
  "\313o\377]\213\"\3773i\2\2522a\5t/[\5d2d\5}:o\7\304r\233/\377\274\321c\377"
  "\264\314M\377\262\312I\377\262\312I\377\262\312I\377\262\312I\377\262\312"
  "I\377\263\313I\377y\240*\377>t\12>>t\14\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  """5n\1\212\307\330\213\377\267\317J\377\272\320O\377\272\320O\377\272\320"
  "O\377\272\320O\377\272\320O\377\271\320M\377\276\323Z\377\315\336y\377\306"
  "\330{\377\245\300]\377\233\271S\377\247\301\\\377\310\332q\377\302\326`\377"
  "\271\320N\377\272\320O\377\272\320O\377\272\320O\377\272\320O\377\272\320"
  "O\377\271\320N\377\266\316O\377@u\13\322\0\0\0\0\7\7\7\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0""5n\0\213\312\333\215\377\276\324O\377\277\325S\377\277"
  "\325R\377\300\325T\377\300\325T\377\300\325T\377\300\325T\377\300\325T\377"
  "\277\325R\377\277\324R\377\302\327Z\377\303\327]\377\301\326X\377\277\324"
  "R\377\277\325S\377\300\325T\377\300\325T\377\300\325T\377\300\325T\377\300"
  "\325T\377\277\325S\377\307\332\\\377c\220\37\377*J\10""4\1\2\0\21\0\0\0\10"
  "\1\1\1\1\0\0\0\0\0\0\0\0\0\0\0\2""5l\1\216\315\336\220\377\305\331U\377\335"
  "\352\230\377\320\340u\377\305\332W\377\306\333X\377\306\333X\377\306\333"
  "X\377\306\333X\377\306\333X\377\306\333X\377\306\333X\377\306\333X\377\306"
  "\333X\377\306\333X\377\306\333X\377\306\333X\377\306\333X\377\306\333X\377"
  "\306\333Y\377\306\332X\377\313\336a\377\206\2537\3770]\6\201\0\0\0!\0\0\0"
  "\32\0\0\0\22\0\0\0\11\1\1\1\1\2\2\2\1\0\0\0\10""3j\0\220\320\340\220\377"
  "\343\356\230\377y\240;\377\301\325\200\377\330\347\200\377\314\337[\377\315"
  "\337]\377\314\337]\377\314\337]\377\314\337]\377\314\337]\377\314\337]\377"
  "\314\337]\377\314\337]\377\314\337]\377\314\337]\377\314\337]\377\314\337"
  "]\377\315\337]\377\314\337]\377\324\345l\377\221\263@\3771a\6\250\0\0\0+"
  "\0\0\0*\0\0\0!\0\0\0\30\0\0\0\17\0\0\0\6\4\4\4\2\0\0\0\12""3i\0\221\332\347"
  "\227\377s\2338\377-T\5p8l\6\312\247\301f\377\350\362\234\377\325\346k\377"
  "\322\345a\377\323\345c\377\322\345c\377\322\345c\377\322\345c\377\322\345"
  "c\377\322\345c\377\322\345c\377\322\345c\377\322\345c\377\322\345b\377\325"
  "\346j\377\334\353x\377y\2411\377-X\5\237\0\0\0""8\0\0\0""5\0\0\0,\0\0\0#"
  "\0\0\0\32\0\0\0\21\0\0\0\10\0\0\0\1\0\0\0\10""9n\6\220i\224/\377.X\5d\0\0"
  "\0)\0\0\0--Z\4\226j\226-\377\326\345\222\377\350\362\230\377\334\353y\377"
  "\330\351h\377\326\350c\377\327\350d\377\327\350e\377\327\350d\377\327\350"
  "d\377\330\351j\377\333\353t\377\346\362\204\377\256\311[\377L~\24\366!<\5"
  "s\0\0\0@\0\0\0;\0\0\0""2\0\0\0*\0\0\0!\0\0\0\30\0\0\0\17\0\0\0\6\0\0\0\0"
  "\0\0\0\2@t\14~8f\11[\0\0\0\32\0\0\0%\0\0\0-\0\0\0""3\22\40\4J0b\1\273k\227"
  ",\377\252\305e\377\327\346\215\377\356\367\235\377\352\364\226\377\350\363"
  "\222\377\351\363\221\377\347\363\216\377\304\330r\377\220\263G\377O\200\26"
  "\364*S\5\234\0\0\0E\0\0\0B\0\0\0;\0\0\0""3\0\0\0+\0\0\0\"\0\0\0\32\0\0\0"
  "\22\0\0\0\11\0\0\0\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\12\0\0\0\22\0\0\0\32\0"
  "\0\0!\0\0\0)\0\0\0/\0\0\0""3\3\4\0<(M\5\177/_\2\2638k\5\325N}\22\345T\203"
  "\27\353J{\20\3432e\3\315,X\3\243\37;\5m\0\0\0;\0\0\0?\0\0\0:\0\0\0""3\0\0"
  "\0-\0\0\0'\0\0\0\40\0\0\0\30\0\0\0\21\0\0\0\10\0\0\0\1\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\5\0\0\0\15\0\0\0\23\0\0\0\32\0\0\0\37\0\0\0"
  "$\0\0\0)\0\0\0.\0\0\0/\0\0\0/\0\0\0/\0\0\0""0\0\0\0""2\0\0\0""3\0\0\0""6"
  "\0\0\0""4\0\0\0""0\0\0\0,\0\0\0(\0\0\0#\0\0\0\35\0\0\0\30\0\0\0\22\0\0\0"
  "\13\0\0\0\3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\11\11\11\0\0\0\0\3\0\0\0\10\0\0\0\14\0\0\0\21\0\0\0\25\0\0\0\31\0\0\0"
  "\34\0\0\0\36\0\0\0\40\0\0\0!\0\0\0\"\0\0\0!\0\0\0\40\0\0\0\36\0\0\0\33\0"
  "\0\0\30\0\0\0\24\0\0\0\20\0\0\0\13\0\0\0\6\1\1\1\3\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0",
};


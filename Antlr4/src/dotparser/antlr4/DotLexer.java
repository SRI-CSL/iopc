// Generated from /Users/iam/Repositories/IOP++/Antlr4/src/dotparser/grammar/Dot.g4 by ANTLR 4.0
package dotparser.antlr4;
import org.antlr.v4.runtime.Lexer;
import org.antlr.v4.runtime.CharStream;
import org.antlr.v4.runtime.Token;
import org.antlr.v4.runtime.TokenStream;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.misc.*;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast"})
public class DotLexer extends Lexer {
	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		T__7=1, T__6=2, T__5=3, T__4=4, T__3=5, T__2=6, T__1=7, T__0=8, ARROW=9, 
		LINE=10, STRICT=11, GRAPH=12, DIGRAPH=13, NODE=14, EDGE=15, SUBGRAPH=16, 
		NUMBER=17, STRING=18, ID=19, HTML_STRING=20, COMMENT=21, LINE_COMMENT=22, 
		PREPROC=23, WS=24;
	public static String[] modeNames = {
		"DEFAULT_MODE"
	};

	public static final String[] tokenNames = {
		"<INVALID>",
		"']'", "'{'", "','", "'['", "':'", "'='", "'}'", "';'", "'->'", "'--'", 
		"STRICT", "GRAPH", "DIGRAPH", "NODE", "EDGE", "SUBGRAPH", "NUMBER", "STRING", 
		"ID", "HTML_STRING", "COMMENT", "LINE_COMMENT", "PREPROC", "WS"
	};
	public static final String[] ruleNames = {
		"T__7", "T__6", "T__5", "T__4", "T__3", "T__2", "T__1", "T__0", "ARROW", 
		"LINE", "STRICT", "GRAPH", "DIGRAPH", "NODE", "EDGE", "SUBGRAPH", "NUMBER", 
		"DIGIT", "STRING", "ID", "LETTER", "HTML_STRING", "TAG", "COMMENT", "LINE_COMMENT", 
		"PREPROC", "WS"
	};


	public DotLexer(CharStream input) {
		super(input);
		_interp = new LexerATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}

	@Override
	public String getGrammarFileName() { return "Dot.g4"; }

	@Override
	public String[] getTokenNames() { return tokenNames; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public String[] getModeNames() { return modeNames; }

	@Override
	public ATN getATN() { return _ATN; }

	@Override
	public void action(RuleContext _localctx, int ruleIndex, int actionIndex) {
		switch (ruleIndex) {
		case 23: COMMENT_action((RuleContext)_localctx, actionIndex); break;

		case 24: LINE_COMMENT_action((RuleContext)_localctx, actionIndex); break;

		case 25: PREPROC_action((RuleContext)_localctx, actionIndex); break;

		case 26: WS_action((RuleContext)_localctx, actionIndex); break;
		}
	}
	private void PREPROC_action(RuleContext _localctx, int actionIndex) {
		switch (actionIndex) {
		case 2: skip();  break;
		}
	}
	private void WS_action(RuleContext _localctx, int actionIndex) {
		switch (actionIndex) {
		case 3: skip();  break;
		}
	}
	private void LINE_COMMENT_action(RuleContext _localctx, int actionIndex) {
		switch (actionIndex) {
		case 1: skip();  break;
		}
	}
	private void COMMENT_action(RuleContext _localctx, int actionIndex) {
		switch (actionIndex) {
		case 0: skip();  break;
		}
	}

	public static final String _serializedATN =
		"\2\4\32\u00ea\b\1\4\2\t\2\4\3\t\3\4\4\t\4\4\5\t\5\4\6\t\6\4\7\t\7\4\b"+
		"\t\b\4\t\t\t\4\n\t\n\4\13\t\13\4\f\t\f\4\r\t\r\4\16\t\16\4\17\t\17\4\20"+
		"\t\20\4\21\t\21\4\22\t\22\4\23\t\23\4\24\t\24\4\25\t\25\4\26\t\26\4\27"+
		"\t\27\4\30\t\30\4\31\t\31\4\32\t\32\4\33\t\33\4\34\t\34\3\2\3\2\3\3\3"+
		"\3\3\4\3\4\3\5\3\5\3\6\3\6\3\7\3\7\3\b\3\b\3\t\3\t\3\n\3\n\3\n\3\13\3"+
		"\13\3\13\3\f\3\f\3\f\3\f\3\f\3\f\3\f\3\r\3\r\3\r\3\r\3\r\3\r\3\16\3\16"+
		"\3\16\3\16\3\16\3\16\3\16\3\16\3\17\3\17\3\17\3\17\3\17\3\20\3\20\3\20"+
		"\3\20\3\20\3\21\3\21\3\21\3\21\3\21\3\21\3\21\3\21\3\21\3\22\5\22y\n\22"+
		"\3\22\3\22\6\22}\n\22\r\22\16\22~\3\22\6\22\u0082\n\22\r\22\16\22\u0083"+
		"\3\22\3\22\7\22\u0088\n\22\f\22\16\22\u008b\13\22\5\22\u008d\n\22\5\22"+
		"\u008f\n\22\3\23\3\23\3\24\3\24\3\24\3\24\7\24\u0097\n\24\f\24\16\24\u009a"+
		"\13\24\3\24\3\24\3\25\3\25\3\25\7\25\u00a1\n\25\f\25\16\25\u00a4\13\25"+
		"\3\26\3\26\3\27\3\27\3\27\7\27\u00ab\n\27\f\27\16\27\u00ae\13\27\3\27"+
		"\3\27\3\30\3\30\7\30\u00b4\n\30\f\30\16\30\u00b7\13\30\3\30\3\30\3\31"+
		"\3\31\3\31\3\31\7\31\u00bf\n\31\f\31\16\31\u00c2\13\31\3\31\3\31\3\31"+
		"\3\31\3\31\3\32\3\32\3\32\3\32\7\32\u00cd\n\32\f\32\16\32\u00d0\13\32"+
		"\3\32\5\32\u00d3\n\32\3\32\3\32\3\32\3\32\3\33\3\33\7\33\u00db\n\33\f"+
		"\33\16\33\u00de\13\33\3\33\3\33\3\33\3\33\3\34\6\34\u00e5\n\34\r\34\16"+
		"\34\u00e6\3\34\3\34\7\u0098\u00b5\u00c0\u00ce\u00dc\35\3\3\1\5\4\1\7\5"+
		"\1\t\6\1\13\7\1\r\b\1\17\t\1\21\n\1\23\13\1\25\f\1\27\r\1\31\16\1\33\17"+
		"\1\35\20\1\37\21\1!\22\1#\23\1%\2\1\'\24\1)\25\1+\2\1-\26\1/\2\1\61\27"+
		"\2\63\30\3\65\31\4\67\32\5\3\2(\4UUuu\4VVvv\4TTtt\4KKkk\4EEee\4VVvv\4"+
		"IIii\4TTtt\4CCcc\4RRrr\4JJjj\4FFff\4KKkk\4IIii\4TTtt\4CCcc\4RRrr\4JJj"+
		"j\4PPpp\4QQqq\4FFff\4GGgg\4GGgg\4FFff\4IIii\4GGgg\4UUuu\4WWww\4DDdd\4"+
		"IIii\4TTtt\4CCcc\4RRrr\4JJjj\3\62;\6C\\aac|\u0082\u0101\4>>@@\5\13\f\17"+
		"\17\"\"\u00f8\2\3\3\2\2\2\2\5\3\2\2\2\2\7\3\2\2\2\2\t\3\2\2\2\2\13\3\2"+
		"\2\2\2\r\3\2\2\2\2\17\3\2\2\2\2\21\3\2\2\2\2\23\3\2\2\2\2\25\3\2\2\2\2"+
		"\27\3\2\2\2\2\31\3\2\2\2\2\33\3\2\2\2\2\35\3\2\2\2\2\37\3\2\2\2\2!\3\2"+
		"\2\2\2#\3\2\2\2\2\'\3\2\2\2\2)\3\2\2\2\2-\3\2\2\2\2\61\3\2\2\2\2\63\3"+
		"\2\2\2\2\65\3\2\2\2\2\67\3\2\2\2\39\3\2\2\2\5;\3\2\2\2\7=\3\2\2\2\t?\3"+
		"\2\2\2\13A\3\2\2\2\rC\3\2\2\2\17E\3\2\2\2\21G\3\2\2\2\23I\3\2\2\2\25L"+
		"\3\2\2\2\27O\3\2\2\2\31V\3\2\2\2\33\\\3\2\2\2\35d\3\2\2\2\37i\3\2\2\2"+
		"!n\3\2\2\2#x\3\2\2\2%\u0090\3\2\2\2\'\u0092\3\2\2\2)\u009d\3\2\2\2+\u00a5"+
		"\3\2\2\2-\u00a7\3\2\2\2/\u00b1\3\2\2\2\61\u00ba\3\2\2\2\63\u00c8\3\2\2"+
		"\2\65\u00d8\3\2\2\2\67\u00e4\3\2\2\29:\7_\2\2:\4\3\2\2\2;<\7}\2\2<\6\3"+
		"\2\2\2=>\7.\2\2>\b\3\2\2\2?@\7]\2\2@\n\3\2\2\2AB\7<\2\2B\f\3\2\2\2CD\7"+
		"?\2\2D\16\3\2\2\2EF\7\177\2\2F\20\3\2\2\2GH\7=\2\2H\22\3\2\2\2IJ\7/\2"+
		"\2JK\7@\2\2K\24\3\2\2\2LM\7/\2\2MN\7/\2\2N\26\3\2\2\2OP\t\2\2\2PQ\t\3"+
		"\2\2QR\t\4\2\2RS\t\5\2\2ST\t\6\2\2TU\t\7\2\2U\30\3\2\2\2VW\t\b\2\2WX\t"+
		"\t\2\2XY\t\n\2\2YZ\t\13\2\2Z[\t\f\2\2[\32\3\2\2\2\\]\t\r\2\2]^\t\16\2"+
		"\2^_\t\17\2\2_`\t\20\2\2`a\t\21\2\2ab\t\22\2\2bc\t\23\2\2c\34\3\2\2\2"+
		"de\t\24\2\2ef\t\25\2\2fg\t\26\2\2gh\t\27\2\2h\36\3\2\2\2ij\t\30\2\2jk"+
		"\t\31\2\2kl\t\32\2\2lm\t\33\2\2m \3\2\2\2no\t\34\2\2op\t\35\2\2pq\t\36"+
		"\2\2qr\t\37\2\2rs\t \2\2st\t!\2\2tu\t\"\2\2uv\t#\2\2v\"\3\2\2\2wy\7/\2"+
		"\2xw\3\2\2\2xy\3\2\2\2y\u008e\3\2\2\2z|\7\60\2\2{}\5%\23\2|{\3\2\2\2}"+
		"~\3\2\2\2~|\3\2\2\2~\177\3\2\2\2\177\u008f\3\2\2\2\u0080\u0082\5%\23\2"+
		"\u0081\u0080\3\2\2\2\u0082\u0083\3\2\2\2\u0083\u0081\3\2\2\2\u0083\u0084"+
		"\3\2\2\2\u0084\u008c\3\2\2\2\u0085\u0089\7\60\2\2\u0086\u0088\5%\23\2"+
		"\u0087\u0086\3\2\2\2\u0088\u008b\3\2\2\2\u0089\u0087\3\2\2\2\u0089\u008a"+
		"\3\2\2\2\u008a\u008d\3\2\2\2\u008b\u0089\3\2\2\2\u008c\u0085\3\2\2\2\u008c"+
		"\u008d\3\2\2\2\u008d\u008f\3\2\2\2\u008ez\3\2\2\2\u008e\u0081\3\2\2\2"+
		"\u008f$\3\2\2\2\u0090\u0091\t$\2\2\u0091&\3\2\2\2\u0092\u0098\7$\2\2\u0093"+
		"\u0094\7^\2\2\u0094\u0097\7$\2\2\u0095\u0097\13\2\2\2\u0096\u0093\3\2"+
		"\2\2\u0096\u0095\3\2\2\2\u0097\u009a\3\2\2\2\u0098\u0099\3\2\2\2\u0098"+
		"\u0096\3\2\2\2\u0099\u009b\3\2\2\2\u009a\u0098\3\2\2\2\u009b\u009c\7$"+
		"\2\2\u009c(\3\2\2\2\u009d\u00a2\5+\26\2\u009e\u00a1\5+\26\2\u009f\u00a1"+
		"\5%\23\2\u00a0\u009e\3\2\2\2\u00a0\u009f\3\2\2\2\u00a1\u00a4\3\2\2\2\u00a2"+
		"\u00a0\3\2\2\2\u00a2\u00a3\3\2\2\2\u00a3*\3\2\2\2\u00a4\u00a2\3\2\2\2"+
		"\u00a5\u00a6\t%\2\2\u00a6,\3\2\2\2\u00a7\u00ac\7>\2\2\u00a8\u00ab\5/\30"+
		"\2\u00a9\u00ab\n&\2\2\u00aa\u00a8\3\2\2\2\u00aa\u00a9\3\2\2\2\u00ab\u00ae"+
		"\3\2\2\2\u00ac\u00aa\3\2\2\2\u00ac\u00ad\3\2\2\2\u00ad\u00af\3\2\2\2\u00ae"+
		"\u00ac\3\2\2\2\u00af\u00b0\7@\2\2\u00b0.\3\2\2\2\u00b1\u00b5\7>\2\2\u00b2"+
		"\u00b4\13\2\2\2\u00b3\u00b2\3\2\2\2\u00b4\u00b7\3\2\2\2\u00b5\u00b6\3"+
		"\2\2\2\u00b5\u00b3\3\2\2\2\u00b6\u00b8\3\2\2\2\u00b7\u00b5\3\2\2\2\u00b8"+
		"\u00b9\7@\2\2\u00b9\60\3\2\2\2\u00ba\u00bb\7\61\2\2\u00bb\u00bc\7,\2\2"+
		"\u00bc\u00c0\3\2\2\2\u00bd\u00bf\13\2\2\2\u00be\u00bd\3\2\2\2\u00bf\u00c2"+
		"\3\2\2\2\u00c0\u00c1\3\2\2\2\u00c0\u00be\3\2\2\2\u00c1\u00c3\3\2\2\2\u00c2"+
		"\u00c0\3\2\2\2\u00c3\u00c4\7,\2\2\u00c4\u00c5\7\61\2\2\u00c5\u00c6\3\2"+
		"\2\2\u00c6\u00c7\b\31\2\2\u00c7\62\3\2\2\2\u00c8\u00c9\7\61\2\2\u00c9"+
		"\u00ca\7\61\2\2\u00ca\u00ce\3\2\2\2\u00cb\u00cd\13\2\2\2\u00cc\u00cb\3"+
		"\2\2\2\u00cd\u00d0\3\2\2\2\u00ce\u00cf\3\2\2\2\u00ce\u00cc\3\2\2\2\u00cf"+
		"\u00d2\3\2\2\2\u00d0\u00ce\3\2\2\2\u00d1\u00d3\7\17\2\2\u00d2\u00d1\3"+
		"\2\2\2\u00d2\u00d3\3\2\2\2\u00d3\u00d4\3\2\2\2\u00d4\u00d5\7\f\2\2\u00d5"+
		"\u00d6\3\2\2\2\u00d6\u00d7\b\32\3\2\u00d7\64\3\2\2\2\u00d8\u00dc\7%\2"+
		"\2\u00d9\u00db\13\2\2\2\u00da\u00d9\3\2\2\2\u00db\u00de\3\2\2\2\u00dc"+
		"\u00dd\3\2\2\2\u00dc\u00da\3\2\2\2\u00dd\u00df\3\2\2\2\u00de\u00dc\3\2"+
		"\2\2\u00df\u00e0\7\f\2\2\u00e0\u00e1\3\2\2\2\u00e1\u00e2\b\33\4\2\u00e2"+
		"\66\3\2\2\2\u00e3\u00e5\t\'\2\2\u00e4\u00e3\3\2\2\2\u00e5\u00e6\3\2\2"+
		"\2\u00e6\u00e4\3\2\2\2\u00e6\u00e7\3\2\2\2\u00e7\u00e8\3\2\2\2\u00e8\u00e9"+
		"\b\34\5\2\u00e98\3\2\2\2\25\2x~\u0083\u0089\u008c\u008e\u0096\u0098\u00a0"+
		"\u00a2\u00aa\u00ac\u00b5\u00c0\u00ce\u00d2\u00dc\u00e6";
	public static final ATN _ATN =
		ATNSimulator.deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
	}
}
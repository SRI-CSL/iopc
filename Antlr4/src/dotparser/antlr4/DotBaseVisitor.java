// Generated from /Users/iam/Repositories/IOP++/Antlr4/src/dotparser/grammar/Dot.g4 by ANTLR 4.0
package dotparser.antlr4;
import org.antlr.v4.runtime.tree.*;
import org.antlr.v4.runtime.Token;
import org.antlr.v4.runtime.ParserRuleContext;

public class DotBaseVisitor<T> extends AbstractParseTreeVisitor<T> implements DotVisitor<T> {
	@Override public T visitAttr_stmt(DotParser.Attr_stmtContext ctx) { return visitChildren(ctx); }

	@Override public T visitPort(DotParser.PortContext ctx) { return visitChildren(ctx); }

	@Override public T visitEdgeop(DotParser.EdgeopContext ctx) { return visitChildren(ctx); }

	@Override public T visitStmt_list(DotParser.Stmt_listContext ctx) { return visitChildren(ctx); }

	@Override public T visitStmt(DotParser.StmtContext ctx) { return visitChildren(ctx); }

	@Override public T visitEdgeRHS(DotParser.EdgeRHSContext ctx) { return visitChildren(ctx); }

	@Override public T visitNode_id(DotParser.Node_idContext ctx) { return visitChildren(ctx); }

	@Override public T visitId(DotParser.IdContext ctx) { return visitChildren(ctx); }

	@Override public T visitSubgraph(DotParser.SubgraphContext ctx) { return visitChildren(ctx); }

	@Override public T visitGraph(DotParser.GraphContext ctx) { return visitChildren(ctx); }

	@Override public T visitA_list(DotParser.A_listContext ctx) { return visitChildren(ctx); }

	@Override public T visitAttr_list(DotParser.Attr_listContext ctx) { return visitChildren(ctx); }

	@Override public T visitEdge_stmt(DotParser.Edge_stmtContext ctx) { return visitChildren(ctx); }

	@Override public T visitNode_stmt(DotParser.Node_stmtContext ctx) { return visitChildren(ctx); }
}
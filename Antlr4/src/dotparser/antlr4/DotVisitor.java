// Generated from /Users/iam/Repositories/IOP++/Antlr4/src/dotparser/grammar/Dot.g4 by ANTLR 4.0
package dotparser.antlr4;
import org.antlr.v4.runtime.tree.*;
import org.antlr.v4.runtime.Token;

public interface DotVisitor<T> extends ParseTreeVisitor<T> {
	T visitAttr_stmt(DotParser.Attr_stmtContext ctx);

	T visitPort(DotParser.PortContext ctx);

	T visitEdgeop(DotParser.EdgeopContext ctx);

	T visitStmt_list(DotParser.Stmt_listContext ctx);

	T visitStmt(DotParser.StmtContext ctx);

	T visitEdgeRHS(DotParser.EdgeRHSContext ctx);

	T visitNode_id(DotParser.Node_idContext ctx);

	T visitId(DotParser.IdContext ctx);

	T visitSubgraph(DotParser.SubgraphContext ctx);

	T visitGraph(DotParser.GraphContext ctx);

	T visitA_list(DotParser.A_listContext ctx);

	T visitAttr_list(DotParser.Attr_listContext ctx);

	T visitEdge_stmt(DotParser.Edge_stmtContext ctx);

	T visitNode_stmt(DotParser.Node_stmtContext ctx);
}
import sys
import ast
import astor

class RemoveDocstrings(ast.NodeTransformer):
    def visit_Expr(self, node):
        if isinstance(node.value, ast.Str):
            return None
        else:
            return node

with open(sys.argv[1]) as f:
    source = f.read()

module = ast.parse(source, '<nofile', 'exec')

print(astor.to_source(RemoveDocstrings().visit(module)))

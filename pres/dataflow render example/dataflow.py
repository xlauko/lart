from IPython.display import display, Javascript, HTML

from enum import Enum, auto
from graphviz import Digraph
from typing import Optional


class NodeKind(Enum):
    Entry = auto()
    Expr = auto()
    Choose = auto()
    Error = auto()


NodeId = str


class Node:
    def __init__(self, id: NodeId, label: str, kind: NodeKind) -> None:
        self.id: NodeId = id
        self.label: str = label
        self.kind: NodeKind = kind


class Edge:
    def __init__(self, src: NodeId, dst: NodeId) -> None:
        self.src: NodeId = src
        self.dst: NodeId = dst


class Dataflow:
    def __init__(self) -> None:
        self.nodes: List[Node] = []
        self.edges: List[Edge] = []
        self.prev: Optional[NodeId] = None

    def set_node_style(self,
                       dot: Digraph,
                       kind: NodeKind,
                       id: NodeId) -> None:
        if kind == NodeKind.Entry:
            dot.attr('node', shape='none')
        if kind == NodeKind.Expr:
            if id == self.prev:
                dot.attr('node', shape='box', style='filled', color='lightgrey')
            else:
                dot.attr('node', shape='box')
        if kind == NodeKind.Choose:
            if id == self.prev:
                dot.attr('node', shape='diamond', style='filled', color='lightgrey')
            else:
                dot.attr('node', shape='diamond')

    def render_edge(self, dot: Digraph, e: Edge) -> None:
        dot.edge(e.src, e.dst)

    def render_node(self, dot: Digraph, n: Node) -> Node:
        self.set_node_style(dot, n.kind, n.id)
        dot.node(n.id, n.label)

    def render(self) -> None:
        dot = Digraph()

        for n in self.nodes:
            self.render_node(dot, n)
        for e in self.edges:
            self.render_edge(dot, e)

        draw_dataflow_graph(dot_to_string(dot))

    def add_edge(self, e: Edge) -> None:
        self.edges.append(e)

    def add_node(self, n: Node) -> None:
        self.nodes.append(n)

        if self.prev:
            self.add_edge(Edge(self.prev, n.id))
        self.prev = n.id


def draw_dataflow_graph(dot: str) -> None:
    display(
        Javascript("""
        d3.select("#graph").graphviz()
            .renderDot('%s');
        """ % dot))


def dot_to_string(dot: Digraph) -> str:
    return str(dot.source.replace('\n', '\\n').replace('\t', '\\t'))

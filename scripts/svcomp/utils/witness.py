#!/usr/bin/env python3

# Inspired by Symbiotic Witness generator
no_lxml = False
try:
    from lxml import etree as ET
except ImportError:
    no_lxml = True
if no_lxml:
    # if this fails, then we're screwed, so let the script die
    from xml.etree import ElementTree as ET

from . log import logger
from . model import Model, ModelVar

import os
import datetime
import hashlib

class Node:
    def __init__(self, id):
        self.id = id
        self.data = None

    def to_xml(self, parent):
        elem = ET.SubElement(parent, 'node', id=f'{self.id}')
        if self.data:
            self.data.to_xml(elem)

class Data:
    def __init__(self, key, value):
        self.key = key
        self.value = value

    def to_xml(self, parent):
        data = ET.SubElement(parent, 'data', key=f'{self.key}')
        data.text = self.value


class NondetEdge:
    def __init__(self, src, dst, var):
        self.src = src
        self.dst = dst
        self.var = var

    def to_xml(self, graph):
        edge = ET.SubElement(graph, 'edge', source=self.src, target=self.dst)
        ET.SubElement(edge, 'data', key='assumption').text = \
            f'\\result=={self.var.value.value}'
        ET.SubElement(edge, 'data', key='assumption.resultfunction').text = self.var.call
        ET.SubElement(edge, 'data', key='startline').text = self.var.line

class WitnessWriter(object):

    def __init__(self, result, cfg, model, path, correctness):
        self.result = result
        self.correctness = correctness
        self.model = model
        self.path = path
        self.arch = str(cfg.architecture) + 'bit'
        self.property_strings = cfg.property_strings
        self.source = cfg.benchmark

        ET.register_namespace("xsi",
                              "http://www.w3.org/2001/XMLSchema-instance")

        if no_lxml:
            self.root = ET.Element('graphml')
        else:
            ns = {None: 'http://graphml.graphdrawing.org/xmlns',
                  "xsi": "http://www.w3.org/2001/XMLSchema-instance"}
            self.root = ET.Element('graphml', nsmap=ns)

        self.graphml = ET.ElementTree(self.root)
        self.graph = ET.SubElement(self.root, 'graph', edgedefault="directed")

    def add_info(self):
        assert self.root is not None

        config = {
            'witness-type': 'correctness_witness' if self.correctness else 'violation_witness',
            'sourcecodelang': 'C',
            'producer': 'LART',
            'programfile': self.source,
            'programhash': sha256sum(self.source),
            'architecture': self.arch,
            'creationtime': str(datetime.datetime.now().astimezone().replace(microsecond=0).isoformat())
        }

        for key, value in config.items():
            ET.SubElement(self.graph, 'data', key=key).text = value
            self.add_graph_key(key)

        self.add_graph_key('specification')
        for prp in self.property_strings:
            ET.SubElement(self.graph, 'data', key='specification').text = str(prp)

        self.add_node_key('entry')
        self.add_node_key('violation')

        self.add_edge_key('assumption')
        self.add_edge_key('assumption.resultfunction')
        self.add_edge_key('startline')

    def add_edge_key(self, name):
        e = ET.SubElement(self.root, 'key', id=name)
        e.set('for', 'edge')
        e.set('attr.type', 'string')
        e.set('attr.name', name)

    def add_node_key(self, name):
        e = ET.SubElement(self.root, 'key', id=name)
        e.set('for', 'node')
        e.set('attr.type', 'string')
        e.set('attr.name', name)

    def add_graph_key(self, name: str):
        e = ET.SubElement(self.root, 'key', id=name)
        e.set('for', 'graph')
        e.set('attr.type', 'string')
        e.set('attr.name', name)

    def trivial(self):
        logger().info("generating trivial witness")
        entry = Node("entry")
        entry.data = Data("entry", "true")
        entry.to_xml(self.graph)
        self.add_info()

    def parse_error(self):
        logger().info("generating error witness")
        entry = Node("entry")
        entry.data = Data("entry", "true")
        entry.to_xml(self.graph)

        last = entry
        for idx, var in enumerate(self.model.vars):
            if var.value is None:
                continue # FIXME skip invalid model values
            node = Node(str(idx))
            node.to_xml(self.graph)
            edge = NondetEdge(last.id, node.id, var)
            edge.to_xml(self.graph)
            last = node

        error = Node("error")
        error.data = Data("violation", "true")
        error.to_xml(self.graph)

        ET.SubElement(self.graph, 'edge', source=last.id, target="error")
        self.add_info()


    def dump(self):
        if self.correctness:
            self.trivial()
        else:
            self.parse_error()

        if no_lxml:
            print(ET.tostring(self.root).decode('utf-8'))
        else:
            print(ET.tostring(self.root, pretty_print=True).decode('utf-8'))


    def write(self, to):
        file = os.getcwd() + "/" + to
        logger().info(f"writing witness to {file}")
        if self.correctness:
            self.trivial()
        else:
            self.parse_error()

        et = self.graphml
        if no_lxml:
            et.write(file, encoding='UTF-8', method="xml",
                     xml_declaration=True)
        else:
            et.write(file, encoding='UTF-8', method="xml",
                     pretty_print=True, xml_declaration=True)


def sha256sum(filename):
    h  = hashlib.sha256()
    b  = bytearray(128*1024)
    mv = memoryview(b)
    with open(filename, 'rb', buffering=0) as f:
        for n in iter(lambda : f.readinto(mv), 0):
            h.update(mv[:n])
    return h.hexdigest()

#!/usr/bin/env python3

import logging
import sys

def logger():
    return logging.getLogger('root')


def error(msg: str) -> None:
    logger().error(msg)
    sys.exit()

def setup_logger():
    formatter = logging.Formatter(fmt='%(levelname)s: %(message)s')

    handler = logging.StreamHandler()
    handler.setFormatter(formatter)

    logger = logging.getLogger('root')
    logger.addHandler(handler)
    return logger

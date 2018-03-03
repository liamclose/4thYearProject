# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('peer-to-peer', ['core'])
    module.source = [
        'model/peer-to-peer.cc',
        'helper/peer-to-peer-helper.cc',
        'model/peer-to-peer-server.cc',
       
        ]

    module_test = bld.create_ns3_module_test_library('peer-to-peer')
    module_test.source = [
        'test/peer-to-peer-test-suite.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'peer-to-peer'
    headers.source = [
        'model/peer-to-peer.h',
        'helper/peer-to-peer-helper.h',
        'model/peer-to-peer-server.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()


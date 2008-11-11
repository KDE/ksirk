# FIND_PACKAGE(QCA2 REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)

set(libiris_SRCS
	#irisnet/appledns/appledns.cpp
	#irisnet/appledns/qdnssd.cpp
	irisnet/corelib/irisnetglobal.cpp
	irisnet/corelib/irisnetplugin.cpp
	irisnet/corelib/jdnsshared.cpp
	irisnet/corelib/netavailability.cpp
	irisnet/corelib/netinterface.cpp
	irisnet/corelib/netnames_jdns.cpp
	irisnet/corelib/netnames.cpp
	irisnet/corelib/objectsession.cpp
	irisnet/noncore/legacy/safedelete.cpp
	irisnet/noncore/legacy/ndns.cpp
	irisnet/noncore/legacy/servsock.cpp
	irisnet/noncore/legacy/srvresolver.cpp
	irisnet/noncore/processquit.cpp
	jdns/jdns_mdnsd.c
	jdns/jdns_packet.c
	jdns/jdns_sys.c
	jdns/jdns_util.c
	jdns/jdns.c
	jdns/qjdns_sock.cpp
	jdns/qjdns.cpp
	libidn/nfkc.c
	libidn/profiles.c
	libidn/rfc3454.c
	libidn/stringprep.c
  xmpp/base/randomnumbergenerator.cpp
  xmpp/base64/base64.cpp
  xmpp/cutestuff/bsocket.cpp
	xmpp/cutestuff/bytestream.cpp
	xmpp/cutestuff/httpconnect.cpp
	xmpp/cutestuff/httppoll.cpp
	xmpp/cutestuff/socks.cpp
  xmpp/sasl/digestmd5proplist.cpp
  xmpp/sasl/digestmd5response.cpp
  xmpp/sasl/plainmessage.cpp
  xmpp/zlib/zlibcompressor.cpp
  xmpp/zlib/zlibdecompressor.cpp
  xmpp/jid/jid.cpp
# 	xmpp/xmpp-core/compress.cpp
	xmpp/xmpp-core/compressionhandler.cpp
	xmpp/xmpp-core/connector.cpp
	xmpp/xmpp-core/parser.cpp
	xmpp/xmpp-core/protocol.cpp
	xmpp/xmpp-core/securestream.cpp
	xmpp/xmpp-core/simplesasl.cpp
	xmpp/xmpp-core/stream.cpp
	xmpp/xmpp-core/tlshandler.cpp
	xmpp/xmpp-core/xmlprotocol.cpp
  xmpp/xmpp-core/xmpp_stanza.cpp
  xmpp/xmpp-im/client.cpp
	xmpp/xmpp-im/filetransfer.cpp
	xmpp/xmpp-im/s5b.cpp
	xmpp/xmpp-im/types.cpp
	xmpp/xmpp-im/xmpp_discoinfotask.cpp
	xmpp/xmpp-im/xmpp_discoitem.cpp
	xmpp/xmpp-im/xmpp_features.cpp
	xmpp/xmpp-im/xmpp_ibb.cpp
	xmpp/xmpp-im/xmpp_task.cpp
	xmpp/xmpp-im/xmpp_tasks.cpp
	xmpp/xmpp-im/xmpp_vcard.cpp
	xmpp/xmpp-im/xmpp_xdata.cpp
  xmpp/xmpp-im/xmpp_xmlcommon.cpp
  )
if(UNIX)
  set(libiris_SRCS ${libiris_SRCS}
		irisnet/corelib/netinterface_unix.cpp
	)
endif(UNIX)
if(WIN32)
  set(libiris_SRCS ${libiris_SRCS}
		irisnet/corelib/netinterface_win.cpp
	)
endif(WIN32)

include_directories(
	#irisnet/appledns
	irisnet/corelib
	irisnet/noncore
	irisnet/noncore/legacy
	jdns
	libidn
	xmpp/cutestuff
	xmpp/jid
	xmpp/xmpp-core
	xmpp/xmpp-im
  ${QCA2_INCLUDE_DIR}
  )

kde4_moc_headers(iris_ksirk
irisnet/corelib/jdnsshared.h
irisnet/corelib/netavailability.h
irisnet/corelib/netinterface.h
irisnet/corelib/netnames.h
irisnet/corelib/objectsession.h
irisnet/noncore/processquit.h
jdns/qjdns.h
xmpp/base/randrandomnumbergenerator.h
xmpp/base64/base64.h
xmpp/cutestuff/bsocket.h
xmpp/cutestuff/bytestream.h
xmpp/cutestuff/httpconnect.h
xmpp/cutestuff/socks.h
xmpp/cutestuff/httppoll.h
xmpp/sasl/digestmd5proplist.cpp
xmpp/sasl/digestmd5response.cpp
xmpp/sasl/plainmessage.cpp
xmpp/xmpp-core/xmpp.h
xmpp/xmpp-core/xmpp_clientstream.h
xmpp/xmpp-core/xmpp_stream.h
xmpp/xmpp-core/securestream.h
xmpp/xmpp-core/simplesasl.h
xmpp/xmpp-im/s5b.h
xmpp/xmpp-im/xmpp_client.h
xmpp/xmpp-im/xmpp_task.h
xmpp/zlib/common.h
xmpp/zlib/zlibcompressor.h
xmpp/zlib/zlibdecompressor.h
)


kde4_add_library(iris_ksirk SHARED ${libiris_SRCS})

set(iris_ksirk_libs ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY}
${IDN_LIBRARY} ${QCA2_LIBRARIES} ${ZLIB_LIBRARIES})
if(WIN32)
  set(iris_ksirk_libs ${iris_ksirk_libs} ws2_32)
endif(WIN32)
  
if(HAVE_NSL_LIBRARY)
    # This is probably Solaris, and libiris needs to link
    # to libnsl for gethostbyname
  set(iris_ksirk_libs ${iris_ksirk_libs} nsl)
endif(HAVE_NSL_LIBRARY)
target_link_libraries(iris_ksirk ${iris_ksirk_libs})
    
set_target_properties(iris_ksirk PROPERTIES VERSION 2.0.0 SOVERSION 2 )
install(TARGETS iris_ksirk ${INSTALL_TARGETS_DEFAULT_ARGS})
    
    ########### install files ###############
    
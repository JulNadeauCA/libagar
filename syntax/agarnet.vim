" Vim syntax file
" Language:     LibAgar - Agar-Net C API
" URL:
" https://github.com/JulNadeauCA/libagar/blob/master/syntax/agarnet.vim
" Maintainer:   Julien Nadeau Carriere <vedge@csoft.net>
" Last Change:  2023 January 09

if !exists("c_no_agar_net") || exists("c_agar_net_typedefs")
  " net/net.h
  syn keyword cType AG_NetAcceptFilter AG_NetAddr AG_NetAddrList
  syn keyword cType AG_NetSocket AG_NetSocketSet AG_NetOps
  syn keyword cConstant AG_NET_AF_NONE AG_NET_LOCAL AG_NET_INET4 AG_NET_INET6
  syn keyword cConstant AG_NET_SOCKET_NONE AG_NET_STREAM AG_NET_DGRAM AG_NET_RAW
  syn keyword cConstant AG_NET_RDM AG_NET_SEQPACKET AG_NET_SO_NONE AG_NET_DEBUG
  syn keyword cConstant AG_NET_REUSEADDR AG_NET_KEEPALIVE AG_NET_DONTROUTE
  syn keyword cConstant AG_NET_BROADCAST AG_NET_BINDANY AG_NET_SNDBUF AG_NET_RCVBUF
  syn keyword cConstant AG_NET_SNDLOWAT AG_NET_RCVLOWAT AG_NET_SNDTIMEO AG_NET_RCVTIMEO
  syn keyword cConstant AG_NET_BACKLOG AG_NET_OOBINLINE AG_NET_REUSEPORT AG_NET_TIMESTAMP
  syn keyword cConstant AG_NET_NOSIGPIPE AG_NET_LINGER AG_NET_ACCEPTFILTER AG_NET_LAST
  syn keyword cConstant AG_NET_ADDRCONFIG AG_NET_NUMERIC_HOST AG_NET_NUMERIC_PORT
  syn keyword cConstant AG_NET_SOCKET_BOUND AG_NET_SOCKET_CONNECTED AG_NET_POLL_READ
  syn keyword cConstant AG_NET_POLL_WRITE AG_NET_POLL_EXCEPTIONS
endif

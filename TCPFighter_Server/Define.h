#pragma once

#define dfNETWORK_PORT              21201
#define dfPACKET_CODE               0x89

#define dfSCREEN_WIDTH              640
#define dfSCREEN_HEIGHT             480

#define dfRANGE_MOVE_LEFT           0
#define dfRANGE_MOVE_RIGHT          6400
#define dfRANGE_MOVE_TOP            0
#define dfRANGE_MOVE_BOTTOM         6400

#define dfSPEED_PLAYER_X            3
#define dfSPEED_PLAYER_Y            2

#define dfSECTOR_SIZE_X             256
#define dfSECTOR_SIZE_Y             256
#define dfSECTOR_MAX_X              (dfRANGE_MOVE_RIGHT / dfSECTOR_SIZE_X)
#define dfSECTOR_MAX_Y              (dfRANGE_MOVE_BOTTOM / dfSECTOR_SIZE_Y)

#define dfERROR_RANGE               50

#define dfATTACK1_RANGE_X           80
#define dfATTACK1_RANGE_Y           10
#define dfATTACK1_DAMAGE            50

#define dfATTACK2_RANGE_X           90
#define dfATTACK2_RANGE_Y           10
#define dfATTACK2_DAMAGE            80

#define dfATTACK3_RANGE_X           100
#define dfATTACK3_RANGE_Y           20
#define dfATTACK3_DAMAGE            100

#define dfINIT_HP                   100

#define dfFRAME_INTERVAL            40
#define dfNETWORK_PACKET_RECV_TIMEOUT  30000

#define dfACTION_STAND              0xff
#define dfACTION_MOVE_LL            0
#define dfACTION_MOVE_LU            1
#define dfACTION_MOVE_UU            2
#define dfACTION_MOVE_RU            3
#define dfACTION_MOVE_RR            4
#define dfACTION_MOVE_RD            5
#define dfACTION_MOVE_DD            6
#define dfACTION_MOVE_LD            7

#define dfRINGBUFFER_SIZE           4096
#define dfSENDBUFFER_SIZE           65536
#define dfRECVBUFFER_SIZE           16384
#define dfPACKET_BUFFER_SIZE        512

#define dfSESSION_MAX           15000
#define dfPACKET_PROC_LIMIT     100

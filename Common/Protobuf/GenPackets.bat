pushd %~dp0
protoc.exe -I=./ --cpp_out=./ ./Protocol.proto
protoc.exe -I=./ --cpp_out=./ ./Enum.proto
protoc.exe -I=./ --cpp_out=./ ./Struct.proto

GenPackets.exe --path=./Protocol.proto --output=ClientPacketHandler --recv=C_ --send=S_
GenPackets.exe --path=./Protocol.proto --output=ServerPacketHandler --recv=S_ --send=C_

IF ERRORLEVEL 1 PAUSE

XCOPY Enum.pb.h "../../GameServer" /E /Y /I
XCOPY Enum.pb.cc "../../GameServer" /E /Y /I
XCOPY Struct.pb.h "../../GameServer" /E /Y /I
XCOPY Struct.pb.cc "../../GameServer" /E /Y /I
XCOPY Protocol.pb.h "../../GameServer" /E /Y /I
XCOPY Protocol.pb.cc "../../GameServer" /E /Y /I
XCOPY ClientPacketHandler.h "../../GameServer" /E /Y /I

XCOPY Enum.pb.h "../../../console-game/Game/Source/Protocol" /E /Y /I
XCOPY Enum.pb.cc "../../../console-game/Game/Source/Protocol" /E /Y /I
XCOPY Struct.pb.h "../../../console-game/Game/Source/Protocol" /E /Y /I
XCOPY Struct.pb.cc "../../../console-game/Game/Source/Protocol" /E /Y /I
XCOPY Protocol.pb.h "../../../console-game/Game/Source/Protocol" /E /Y /I
XCOPY Protocol.pb.cc "../../../console-game/Game/Source/Protocol" /E /Y /I
XCOPY ServerPacketHandler.h "../../../console-game/Game/Source/Protocol" /E /Y /I

XCOPY Source\*.hpp "../../Libraries/Include/CraftEngine" /E /Y /I


DEL /Q /F *.pb.h
DEL /Q /F *.pb.cc
DEL /Q /F *.h

PAUSE
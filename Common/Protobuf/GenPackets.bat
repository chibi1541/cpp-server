pushd %~dp0
protoc.exe -I=./ --cpp_out=./ ./Protocol.proto
protoc.exe -I=./ --cpp_out=./ ./Enum.proto
protoc.exe -I=./ --cpp_out=./ ./Struct.proto

GenPackets.exe --path=./Protocol.proto --output=ClientPacketHandler --recv=C_ --send=S_
:: GenPackets.exe --path=./Protocol.proto --output=ServerPacketHandler --recv=S_ --send=C_

IF ERRORLEVEL 1 PAUSE

XCOPY /Y Enum.pb.h "../../GameServer" /E /Y /I
XCOPY /Y Enum.pb.cc "../../GameServer" /E /Y /I
XCOPY /Y Struct.pb.h "../../GameServer" /E /Y /I
XCOPY /Y Struct.pb.cc "../../GameServer" /E /Y /I
XCOPY /Y Protocol.pb.h "../../GameServer" /E /Y /I
XCOPY /Y Protocol.pb.cc "../../GameServer" /E /Y /I
XCOPY /Y ClientPacketHandler.h "../../GameServer" /E /Y /I

:: XCOPY /Y Enum.pb.h "../../DummyClient" /E /Y /I
:: XCOPY /Y Enum.pb.cc "../../DummyClient" /E /Y /I
:: XCOPY /Y Struct.pb.h "../../DummyClient" /E /Y /I
:: XCOPY /Y Struct.pb.cc "../../DummyClient" /E /Y /I
:: XCOPY /Y Protocol.pb.h "../../DummyClient" /E /Y /I
:: XCOPY /Y Protocol.pb.cc "../../DummyClient" /E /Y /I
:: XCOPY /Y ServerPacketHandler.h "../../DummyClient" /E /Y /I

DEL /Q /F *.pb.h
DEL /Q /F *.pb.cc
DEL /Q /F *.h

PAUSE
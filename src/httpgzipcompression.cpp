/**
 * @file
 * @author Sergey Alikin alikin.sergey@gmail.com
 */
#include "httpgzipcompression.h"
#include <QDataStream>


QByteArray HttpGZipCompression::compressData(const QByteArray &data) {
    return header() + compress(data) + footer(data);
}


QByteArray HttpGZipCompression::header() {
    return QByteArray::fromHex("1f8b080000000000000b");
}


QByteArray HttpGZipCompression::compress(const QByteArray &sourceData) {
    QByteArray compressedData = qCompress(sourceData);
    compressedData.remove(0, 6);
    compressedData.chop(4);
    return compressedData;
}


QByteArray HttpGZipCompression::footer(const QByteArray &sourceData) {
    QByteArray footer;
    QDataStream footerStream(&footer, QIODevice::WriteOnly);
    footerStream.setByteOrder(QDataStream::LittleEndian);
    footerStream << crc32(sourceData) << quint32(sourceData.size());
    return footer;
}


quint32 HttpGZipCompression::updateCRC32(unsigned char ch, quint32 crc) {
    return (crc_32_tab[((crc) ^ ((quint8)ch)) & 0xff] ^ ((crc) >> 8));
}


quint32 HttpGZipCompression::crc32(const QByteArray& data) {
    quint32 crc = 0xFFFFFFFF;
    for (int i=0; i<data.size(); i++) {
        crc = updateCRC32(data[i], crc);
        }
    return crc;
}


/*
  Copyright (C) 2008-2020 The Communi Project

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ircmessagedecoder_p.h"
#include "irccore_p.h"
#include <IrcGlobal>
#include <QSet>

#ifndef IRC_DOXYGEN

IRC_BEGIN_NAMESPACE

IRC_CORE_EXPORT bool irc_is_supported_encoding(const QByteArray& encoding)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

    static QSet<QByteArray> codecs = IrcPrivate::listToSet(QTextCodec::availableCodecs());
    return codecs.contains(encoding);

#else

    QSet<QByteArray> codecs;
    for(int i = 0; i <= QStringConverter::Encoding::LastEncoding; ++i)
    {
        codecs.insert(QStringConverter::nameForEncoding((QStringConverter::Encoding)i));
    }

    return codecs.contains(encoding);

#endif
}

IrcMessageDecoder::IrcMessageDecoder()
{
    initialize();
}

IrcMessageDecoder::~IrcMessageDecoder()
{
    uninitialize();
}

QString IrcMessageDecoder::decode(const QByteArray& data, const QByteArray& encoding) const
{
    if (data.isEmpty())
        return QString();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

    static const QTextCodec *utf8Codec = QTextCodec::codecForName("UTF-8");
    if (utf8Codec) {
        QTextCodec::ConverterState state;
        QString utf8 = utf8Codec->toUnicode(data, data.length(), &state);
        if (state.invalidChars == 0)
            return utf8;
    }

    QTextCodec *defaultCodec = QTextCodec::codecForName(encoding);
    if (!defaultCodec)
        defaultCodec = QTextCodec::codecForName("UTF-8");

    QTextCodec* codec = QTextCodec::codecForUtfText(data, defaultCodec);
    Q_ASSERT(codec);
    return codec->toUnicode(data);

#else

    auto toCodec = QStringDecoder(QStringConverter::Encoding::Utf8);
    return toCodec(data);

#endif

}
#endif // IRC_DOXYGEN

IRC_END_NAMESPACE

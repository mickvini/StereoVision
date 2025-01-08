#include "Encode.h"


void EncodeWithFFmpeg(const cv::Mat& frame, int compressionQuality) {
    // Инициализация кодека
    AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        std::cerr << "H.264 кодек не найден!" << std::endl;
        return;
    }

    AVCodecContext* context = avcodec_alloc_context3(codec);
    if (!context) {
        std::cerr << "Ошибка выделения контекста кодека!" << std::endl;
        return;
    }

    // Настройка параметров кодека
    context->width = frame.cols;
    context->height = frame.rows;
    context->time_base = { 1, 25 }; // Частота кадров 25 fps
    context->pix_fmt = AV_PIX_FMT_YUV420P;

    // Установка параметров сжатия
    context->bit_rate = compressionQuality * 1000; // Пример: качество влияет на битрейт
    context->gop_size = 10; // Количество кадров между ключевыми кадрами
    context->max_b_frames = 1; // Количество B-кадров

    if (avcodec_open2(context, codec, nullptr) < 0) {
        std::cerr << "Ошибка открытия кодека!" << std::endl;
        avcodec_free_context(&context);
        return;
    }

    // Преобразование кадра в формат YUV420P
    SwsContext* swsCtx = sws_getContext(
        frame.cols, frame.rows, AV_PIX_FMT_BGR24, // Исходный формат
        frame.cols, frame.rows, AV_PIX_FMT_YUV420P, // Целевой формат
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    if (!swsCtx) {
        std::cerr << "Ошибка создания контекста преобразования!" << std::endl;
        avcodec_free_context(&context);
        return;
    }

    // Создание структуры кадра
    AVFrame* avFrame = av_frame_alloc();
    avFrame->width = frame.cols;
    avFrame->height = frame.rows;
    avFrame->format = AV_PIX_FMT_YUV420P;

    if (av_frame_get_buffer(avFrame, 32) < 0) {
        std::cerr << "Ошибка выделения буфера для кадра!" << std::endl;
        av_frame_free(&avFrame);
        sws_freeContext(swsCtx);
        avcodec_free_context(&context);
        return;
    }

    uint8_t* srcData[1] = { frame.data };
    int srcStride[1] = { static_cast<int>(frame.step) };

    sws_scale(
        swsCtx, srcData, srcStride, 0, frame.rows,
        avFrame->data, avFrame->linesize
    );

    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        std::cerr << "Ошибка выделения AVPacket." << std::endl;
        av_frame_free(&avFrame);
        sws_freeContext(swsCtx);
        avcodec_free_context(&context);
        return;
    }

    int ret = avcodec_send_frame(context, avFrame);
    if (ret < 0) {
        char errorBuf[256];
        av_strerror(ret, errorBuf, sizeof(errorBuf));
        std::cerr << "Ошибка отправки кадра: " << errorBuf << std::endl;
        av_packet_free(&packet);
        av_frame_free(&avFrame);
        sws_freeContext(swsCtx);
        avcodec_free_context(&context);
        return;
    }

    AVFormatContext* formatCtx = nullptr;
    avformat_alloc_output_context2(&formatCtx, nullptr, nullptr, "output.jpg");
    if (!formatCtx) {
        std::cerr << "Ошибка создания AVFormatContext!" << std::endl;
        av_packet_free(&packet);
        av_frame_free(&avFrame);
        sws_freeContext(swsCtx);
        avcodec_free_context(&context);
        return;
    }

    AVStream* stream = avformat_new_stream(formatCtx, codec);
    if (!stream) {
        std::cerr << "Ошибка создания потока!" << std::endl;
        avformat_free_context(formatCtx);
        av_packet_free(&packet);
        av_frame_free(&avFrame);
        sws_freeContext(swsCtx);
        avcodec_free_context(&context);
        return;
    }

    stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    stream->codecpar->codec_id = AV_CODEC_ID_H264;
    stream->codecpar->width = frame.cols;
    stream->codecpar->height = frame.rows;
    stream->codecpar->format = AV_PIX_FMT_YUV420P;
    stream->time_base = { 1, 25 };
    avcodec_parameters_from_context(stream->codecpar, context);

    if (!(formatCtx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&formatCtx->pb, "output.jpg", AVIO_FLAG_WRITE) < 0) {
            std::cerr << "Ошибка открытия файла для записи!" << std::endl;
            avformat_free_context(formatCtx);
            av_packet_free(&packet);
            av_frame_free(&avFrame);
            sws_freeContext(swsCtx);
            avcodec_free_context(&context);
            return;
        }
    }

    if (avformat_write_header(formatCtx, nullptr) < 0) {
        std::cerr << "Ошибка записи заголовка файла!" << std::endl;
        avio_close(formatCtx->pb);
        avformat_free_context(formatCtx);
        av_packet_free(&packet);
        av_frame_free(&avFrame);
        sws_freeContext(swsCtx);
        avcodec_free_context(&context);
        return;
    }

    // Финализация кодирования
    avcodec_send_frame(context, nullptr);
    while (avcodec_receive_packet(context, packet) == 0) {
        std::cout << "Закодирован пакет при финализации, размер: " << packet->size << " байт" << std::endl;
        av_interleaved_write_frame(formatCtx, packet);
        av_packet_unref(packet);
    }

    av_write_trailer(formatCtx);
    avio_close(formatCtx->pb);
    avformat_free_context(formatCtx);

    av_packet_free(&packet);
    av_frame_free(&avFrame);
    sws_freeContext(swsCtx);
    avcodec_free_context(&context);
}


void CompressAndSaveImage(const cv::Mat& frame, const std::string& outputFile, int quality) {
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_params.push_back(quality); // Качество (0-100)

    if (!cv::imwrite(outputFile, frame, compression_params)) {
        std::cerr << "Ошибка записи изображения!" << std::endl;
    }
}
void DecodeAndDisplayFrame(const std::string& filename) {
    // Инициализация FFmpeg
    av_register_all();

    // Открытие файла
    AVFormatContext* formatCtx = nullptr;
    if (avformat_open_input(&formatCtx, filename.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "Ошибка открытия файла!" << std::endl;
        return;
    }

    // Поиск информации о потоке
    if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
        std::cerr << "Ошибка получения информации о потоке!" << std::endl;
        avformat_close_input(&formatCtx);
        return;
    }

    // Поиск видео потока
    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatCtx->nb_streams; i++) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        std::cerr << "Видео поток не найден!" << std::endl;
        avformat_close_input(&formatCtx);
        return;
    }

    // Получение кодека
    AVCodecParameters* codecParams = formatCtx->streams[videoStreamIndex]->codecpar;
    AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
    if (!codec) {
        std::cerr << "Кодек не найден!" << std::endl;
        avformat_close_input(&formatCtx);
        return;
    }

    // Создание контекста кодека
    AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        std::cerr << "Ошибка выделения контекста кодека!" << std::endl;
        avformat_close_input(&formatCtx);
        return;
    }

    if (avcodec_parameters_to_context(codecCtx, codecParams) < 0) {
        std::cerr << "Ошибка копирования параметров в контекст кодека!" << std::endl;
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return;
    }

    // Открытие кодека
    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        std::cerr << "Ошибка открытия кодека!" << std::endl;
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return;
    }

    // Создание структуры кадра
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "Ошибка выделения кадра!" << std::endl;
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return;
    }

    // Создание структуры пакета
    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        std::cerr << "Ошибка выделения пакета!" << std::endl;
        av_frame_free(&frame);
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return;
    }

    // Создание контекста для преобразования формата
    SwsContext* swsCtx = sws_getContext(
        codecCtx->width, codecCtx->height, codecCtx->pix_fmt,
        codecCtx->width, codecCtx->height, AV_PIX_FMT_BGR24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    if (!swsCtx) {
        std::cerr << "Ошибка создания контекста преобразования!" << std::endl;
        av_packet_free(&packet);
        av_frame_free(&frame);
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return;
    }

    // Чтение кадров
    while (av_read_frame(formatCtx, packet) >= 0) {
        if (packet->stream_index == videoStreamIndex) {
            // Отправка пакета в декодер
            if (avcodec_send_packet(codecCtx, packet) < 0) {
                std::cerr << "Ошибка отправки пакета в декодер!" << std::endl;
                break;
            }

            // Получение кадра из декодера
            while (avcodec_receive_frame(codecCtx, frame) == 0) {
                // Преобразование кадра в формат BGR24
                cv::Mat img(frame->height, frame->width, CV_8UC3);
                uint8_t* dstData[1] = { img.data };
                int dstLinesize[1] = { static_cast<int>(img.step) };
                sws_scale(swsCtx, frame->data, frame->linesize, 0, frame->height, dstData, dstLinesize);

                // Отображение кадра
                cv::flip(img, img, 1);
                cv::imshow("Compressed Frame", img);
                if (cv::waitKey(25) >= 0) {
                    break;
                }
            }
        }

        av_packet_unref(packet);
    }

    // Освобождение ресурсов
    av_packet_free(&packet);
    av_frame_free(&frame);
    sws_freeContext(swsCtx);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&formatCtx);

}
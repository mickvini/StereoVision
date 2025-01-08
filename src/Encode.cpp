#include "Encode.h"


void EncodeWithFFmpeg(const cv::Mat& frame, int compressionQuality) {
    // ������������� ������
    AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        std::cerr << "H.264 ����� �� ������!" << std::endl;
        return;
    }

    AVCodecContext* context = avcodec_alloc_context3(codec);
    if (!context) {
        std::cerr << "������ ��������� ��������� ������!" << std::endl;
        return;
    }

    // ��������� ���������� ������
    context->width = frame.cols;
    context->height = frame.rows;
    context->time_base = { 1, 25 }; // ������� ������ 25 fps
    context->pix_fmt = AV_PIX_FMT_YUV420P;

    // ��������� ���������� ������
    context->bit_rate = compressionQuality * 1000; // ������: �������� ������ �� �������
    context->gop_size = 10; // ���������� ������ ����� ��������� �������
    context->max_b_frames = 1; // ���������� B-������

    if (avcodec_open2(context, codec, nullptr) < 0) {
        std::cerr << "������ �������� ������!" << std::endl;
        avcodec_free_context(&context);
        return;
    }

    // �������������� ����� � ������ YUV420P
    SwsContext* swsCtx = sws_getContext(
        frame.cols, frame.rows, AV_PIX_FMT_BGR24, // �������� ������
        frame.cols, frame.rows, AV_PIX_FMT_YUV420P, // ������� ������
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    if (!swsCtx) {
        std::cerr << "������ �������� ��������� ��������������!" << std::endl;
        avcodec_free_context(&context);
        return;
    }

    // �������� ��������� �����
    AVFrame* avFrame = av_frame_alloc();
    avFrame->width = frame.cols;
    avFrame->height = frame.rows;
    avFrame->format = AV_PIX_FMT_YUV420P;

    if (av_frame_get_buffer(avFrame, 32) < 0) {
        std::cerr << "������ ��������� ������ ��� �����!" << std::endl;
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
        std::cerr << "������ ��������� AVPacket." << std::endl;
        av_frame_free(&avFrame);
        sws_freeContext(swsCtx);
        avcodec_free_context(&context);
        return;
    }

    int ret = avcodec_send_frame(context, avFrame);
    if (ret < 0) {
        char errorBuf[256];
        av_strerror(ret, errorBuf, sizeof(errorBuf));
        std::cerr << "������ �������� �����: " << errorBuf << std::endl;
        av_packet_free(&packet);
        av_frame_free(&avFrame);
        sws_freeContext(swsCtx);
        avcodec_free_context(&context);
        return;
    }

    AVFormatContext* formatCtx = nullptr;
    avformat_alloc_output_context2(&formatCtx, nullptr, nullptr, "output.jpg");
    if (!formatCtx) {
        std::cerr << "������ �������� AVFormatContext!" << std::endl;
        av_packet_free(&packet);
        av_frame_free(&avFrame);
        sws_freeContext(swsCtx);
        avcodec_free_context(&context);
        return;
    }

    AVStream* stream = avformat_new_stream(formatCtx, codec);
    if (!stream) {
        std::cerr << "������ �������� ������!" << std::endl;
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
            std::cerr << "������ �������� ����� ��� ������!" << std::endl;
            avformat_free_context(formatCtx);
            av_packet_free(&packet);
            av_frame_free(&avFrame);
            sws_freeContext(swsCtx);
            avcodec_free_context(&context);
            return;
        }
    }

    if (avformat_write_header(formatCtx, nullptr) < 0) {
        std::cerr << "������ ������ ��������� �����!" << std::endl;
        avio_close(formatCtx->pb);
        avformat_free_context(formatCtx);
        av_packet_free(&packet);
        av_frame_free(&avFrame);
        sws_freeContext(swsCtx);
        avcodec_free_context(&context);
        return;
    }

    // ����������� �����������
    avcodec_send_frame(context, nullptr);
    while (avcodec_receive_packet(context, packet) == 0) {
        std::cout << "����������� ����� ��� �����������, ������: " << packet->size << " ����" << std::endl;
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
    compression_params.push_back(quality); // �������� (0-100)

    if (!cv::imwrite(outputFile, frame, compression_params)) {
        std::cerr << "������ ������ �����������!" << std::endl;
    }
}
void DecodeAndDisplayFrame(const std::string& filename) {
    // ������������� FFmpeg
    av_register_all();

    // �������� �����
    AVFormatContext* formatCtx = nullptr;
    if (avformat_open_input(&formatCtx, filename.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "������ �������� �����!" << std::endl;
        return;
    }

    // ����� ���������� � ������
    if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
        std::cerr << "������ ��������� ���������� � ������!" << std::endl;
        avformat_close_input(&formatCtx);
        return;
    }

    // ����� ����� ������
    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatCtx->nb_streams; i++) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        std::cerr << "����� ����� �� ������!" << std::endl;
        avformat_close_input(&formatCtx);
        return;
    }

    // ��������� ������
    AVCodecParameters* codecParams = formatCtx->streams[videoStreamIndex]->codecpar;
    AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
    if (!codec) {
        std::cerr << "����� �� ������!" << std::endl;
        avformat_close_input(&formatCtx);
        return;
    }

    // �������� ��������� ������
    AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        std::cerr << "������ ��������� ��������� ������!" << std::endl;
        avformat_close_input(&formatCtx);
        return;
    }

    if (avcodec_parameters_to_context(codecCtx, codecParams) < 0) {
        std::cerr << "������ ����������� ���������� � �������� ������!" << std::endl;
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return;
    }

    // �������� ������
    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        std::cerr << "������ �������� ������!" << std::endl;
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return;
    }

    // �������� ��������� �����
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "������ ��������� �����!" << std::endl;
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return;
    }

    // �������� ��������� ������
    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        std::cerr << "������ ��������� ������!" << std::endl;
        av_frame_free(&frame);
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return;
    }

    // �������� ��������� ��� �������������� �������
    SwsContext* swsCtx = sws_getContext(
        codecCtx->width, codecCtx->height, codecCtx->pix_fmt,
        codecCtx->width, codecCtx->height, AV_PIX_FMT_BGR24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    if (!swsCtx) {
        std::cerr << "������ �������� ��������� ��������������!" << std::endl;
        av_packet_free(&packet);
        av_frame_free(&frame);
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return;
    }

    // ������ ������
    while (av_read_frame(formatCtx, packet) >= 0) {
        if (packet->stream_index == videoStreamIndex) {
            // �������� ������ � �������
            if (avcodec_send_packet(codecCtx, packet) < 0) {
                std::cerr << "������ �������� ������ � �������!" << std::endl;
                break;
            }

            // ��������� ����� �� ��������
            while (avcodec_receive_frame(codecCtx, frame) == 0) {
                // �������������� ����� � ������ BGR24
                cv::Mat img(frame->height, frame->width, CV_8UC3);
                uint8_t* dstData[1] = { img.data };
                int dstLinesize[1] = { static_cast<int>(img.step) };
                sws_scale(swsCtx, frame->data, frame->linesize, 0, frame->height, dstData, dstLinesize);

                // ����������� �����
                cv::flip(img, img, 1);
                cv::imshow("Compressed Frame", img);
                if (cv::waitKey(25) >= 0) {
                    break;
                }
            }
        }

        av_packet_unref(packet);
    }

    // ������������ ��������
    av_packet_free(&packet);
    av_frame_free(&frame);
    sws_freeContext(swsCtx);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&formatCtx);

}
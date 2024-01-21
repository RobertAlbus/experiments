#include <cmath>
#include <vector>

#include <matplot/matplot.h>
#include <matplot/util/common.h>

#include <QtCharts/QtCharts>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChartView>
#include <QtWidgets/QWidget>
#include <QVBoxLayout>  // Vertical layout

QWidget* createWaveformPlot(QWidget *parent, const std::vector<float>& data) {
    QChart *chart = new QChart();
    chart->setTitle("2D Waveform Plot");

    QLineSeries *series = new QLineSeries();

    for (size_t i = 0; i < data.size(); ++i) {
        series->append(i, data[i]);
    }

    chart->addSeries(series);
    
    QChartView *chartView = new QChartView(chart, parent);
    chartView->setRenderHint(QPainter::Antialiasing);

    chart->createDefaultAxes();

    chartView->resize(parent->size());
    chartView->show();

    return chartView;
}

std::vector<float> sine_wave(int signalSize, float signalFreq) {
    std::vector<float> signal;
    signal.reserve(signalSize);

    for (int i = 0; i < signalSize; i++) {
        float i_f = ((float)i);
        float sinSignal = std::sin((i_f / signalSize) * matplot::pi * 2.f * signalFreq);
        float envelope = std::lerp(0.f, 1.f, std::fmod(i_f/signalSize*2.f,1.f));

        signal.emplace_back(sinSignal*envelope);
    }

    return signal;
}

enum LIBRARY {
    Qt6,
    Matplot
};
int main(int argc, char *argv[]) {
    int signalSize = 2000;
    float signalFreq = 30.f;

    LIBRARY activeLibrary = LIBRARY::Qt6;

    std::vector<float> signal = sine_wave(signalSize, signalFreq);
    

    std::vector<float> signalRectified;
    std::transform(signal.begin(), signal.end(), std::back_inserter(signalRectified),
            [](float f) { return std::fabs(f); });

    switch (activeLibrary) {
        case LIBRARY::Matplot:
            matplot::subplot(1, 2, 1);
            matplot::plot(signal);
            matplot::subplot(1, 2, 2);
            matplot::plot(signalRectified);
            matplot::show();
            return 0;;
        case LIBRARY::Qt6:
            QApplication app(argc, argv);

            QWidget mainWindow;
            mainWindow.setWindowTitle("Waveform Plotter");

            QVBoxLayout *layout = new QVBoxLayout(&mainWindow); // Create a vertical layout
            QWidget *chart1 = createWaveformPlot(&mainWindow, signal);
            layout->addWidget(chart1);
            QWidget *chart2 = createWaveformPlot(&mainWindow, signalRectified);
            layout->addWidget(chart2);

            mainWindow.setLayout(layout);
            mainWindow.show();
            return app.exec();
    }

}

makeChart('tempChart',0,'Temperatura');
makeChart('humidChart',1,'Wilgotność');
makeChart('pressureChart',2,'Ciśnienie');
makeChart('windChart',3,'Prędkość wiatru');
makeChart('pm1Chart',4,'Pył PM1');
makeChart('pm2Chart',5,'Pył PM2,5');
makeChart('pm10Chart',6,'Pył PM10');

async function makeChart(chartId, dataIndex,label="temperature",interval=10000){
    const loadedData = await loadData();
    var myChart = new Chart(document.getElementById(chartId).getContext('2d'),{
        type: 'line',
        data: {
            labels: loadedData.timeStamp,
            datasets:[{
                label: label,
                data: loadedData.chartData[dataIndex],
                backgroundColor: ['rgba(255, 0, 132, 0.2)'],
                borderColor: ['rgba(255, 99, 132, 1)'],
                borderWidth: 1
            }]
          
        },
    })

    setInterval(async () => {
        const data = await loadData();
        myChart.data.datasets.forEach(d => d.data = data.chartData[dataIndex])
        myChart.labels = data.timeStamp
        myChart.update()
      }, interval)
}

async function loadData() {
    var chartData = [[],[],[],[],[],[],[]];
    var timeStamp = [];
    await $.getJSON('https://api.thingspeak.com/channels/214058/feeds.json?minutes=10', function(data) {
        $.each(data.feeds, function(){
            var temperture = toFixedDecimals(this.field1, 2);
            var humidity = toFixedDecimals(this.field2, 0);
            var pressure = toFixedDecimals(this.field3, 2);
            var wind_speed = toFixedDecimals(this.field4, 2);
            var pm1 = toFixedDecimals(this.field5, 0);
            var pm25 = toFixedDecimals(this.field6, 0);
            var pm10 = toFixedDecimals(this.field7, 0);
            var raw_time = this.created_at;

            if (raw_time){
                var hours;
                var minutes;
                var timewZ = raw_time.split("T").slice(1);
                var timewithoutZ = timewZ[0].split("Z").slice(0);
                var date = new Date(timewithoutZ[0]);
                console.log(date);
            }
            chartData[0].push(temperture);
            chartData[1].push(humidity);
            chartData[2].push(pressure);
            chartData[3].push(wind_speed);
            chartData[4].push(pm1);
            chartData[5].push(pm25);
            chartData[6].push(pm10);
            timeStamp.push(timewithoutZ);
        });
    });
    return {chartData, timeStamp};
    
  }

function toFixedDecimals(number, decimals){
    var fixed = (number / 1000) * 1000;
    fixed = parseFloat(fixed.toFixed(decimals));
    return fixed;
}
const updateFreqency = 10000;

chartIt(updateFreqency);

async function chartIt(interval){
    const loadedData = await loadData();
    var myChart = new Chart(document.getElementById("tempChart").getContext('2d'), {
        type: 'line',
        backgroundColor: 'rgba(255, 251, 230, 0.5)',
        data: {
            labels: loadedData.timeStamp,
            datasets: [{
                label: 'Temperature',
                data: loadedData.chartData,
                backgroundColor: [
                    'rgba(255, 0, 132, 0.2)'
                ],
                borderColor: [
                    'rgba(255, 99, 132, 1)'
                ],
                borderWidth: 1
            }]
        },
        
    })
    
    setInterval(async () => {
      const data = await loadData();
      myChart.data.datasets.forEach(d => d.data = data.chartData)
      myChart.labels = data.timeStamp
      myChart.update()
    }, interval)
}

async function loadData() {
    var chartData = [];
    var timeStamp = [];
    await $.getJSON('https://api.thingspeak.com/channels/214058/fields/1.json?minutes=120', function(data) {
        $.each(data.feeds, function(){
            var value = this.field1;
            var raw_time = this.created_at;
            
             if (value) {
                value = (value / 1000) * 1000;
                value = parseFloat(value.toFixed(2));
            }

            if (raw_time){
                var timewZ = raw_time.split("T").slice(1);
                              
            }
            chartData.push(value);
            timeStamp.push(timewZ);
        });
    });
    return {chartData, timeStamp};
    
  }
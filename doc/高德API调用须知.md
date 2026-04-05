AMap.Weather
天气查询服务，根据城市名称或区域编码返回城市天气预报信息，包括实时天气信息和四天天气预报。

new AMap.Weather()
示例代码：
map.plugin(['AMap.Weather'], function() {
	//构造 Weather 类
	var amapWeather = new AMap.Weather(); 

	//查询实时天气信息，cityName 见 http://restapi.amap.com/v3/config/district?level=city&sublevel=0&extensions=all&output=xml&key=d9fba2f3196b6a4419358693a2b0d9a9
	amapWeather.getLive('北京'); 

	//查询四天预报天气，包括查询当天天气信息
	amapWeather.getForecast('北京'); 

	AMap.event.addListener(amapWeather, "complete", function callback(){
		//当查询成功时触发 complete 事件
	}); 
});
成员函数：
 getLive(city, callback)
 getForecast(city, callback)
事件：
 error
 complete
WeatherLiveResult
Type: Function

参数说明：
err (Object) 正确时为空
LiveData (Object) 返回数据
属性	描述
LiveData.info
类型：String	成功状态文字描述
LiveData.province
类型：String	省份名
LiveData.city
类型：String	城市名
LiveData.adcode
类型：String	区域编码
LiveData.weather
类型：String	天气现象，详见天气现象列表
LiveData.temperature
类型：String	实时气温，单位：摄氏度
LiveData.windDirection
类型：String	风向，风向编码对应描述
LiveData.windPower
类型：Number	风力，风力编码对应风力级别，单位：级
LiveData.humidity
类型：String	空气湿度（百分比）
LiveData.reportTime
类型：String	数据发布的时间
WeatherForecastResult
Type: Function

参数说明：
err (Object) 正确时为空
ForecastData (Object) 返回数据
属性	描述
ForecastData.info
类型：String	成功状态文字描述
ForecastData.province
类型：String	省份名
ForecastData.city
类型：String	城市名
ForecastData.adcode
类型：String	区域编码
ForecastData.reportTime
类型：String	数据发布的时间
ForecastData.forecast
类型：Array	天气预报数组，包括当天至第三天的预报数据
ForecastData.forecast.date
类型：String	日期，格式为“年-月-日”
ForecastData.forecast.week
类型：String	星期
ForecastData.forecast.dayWeather
类型：String	白天天气现象，详见天气现象列表
ForecastData.forecast.nightWeather
类型：String	夜间天气现象，详见天气现象列表
ForecastData.forecast.dayTemp
类型：Number	白天温度
ForecastData.forecast.nightTemp
类型：Number	白天温度
ForecastData.forecast.dayWindDir
类型：String	白天风向
ForecastData.forecast.dayWindPower
类型：String	白天风力
ForecastData.forecast.nightWindPower
类型：String	夜间风力
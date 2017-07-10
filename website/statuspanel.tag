<statuspanel>
	<div>
		<input type="range" min="0" max="100" id="percentage-selector" value={percentage} onchange={updatePercentage}>
		<input type="text" id="percentage-box" value={percentage} onchange={updatePercentage}>
	</div>

	<script>
		var self=this;
		self.percentage=0;
		self.updatePercentage = function(e){
			self.percentage=e.target.value;
			self.setPercentage(self.percentage);

		}

		self.setPercentage = function (percentage) {
	        var req = phonon.ajax({
		            method: 'GET',
		            url: 'http://10.0.0.162/action?cmd=SET-'+percentage,
		            dataType: 'json',
		            success: function(res, xhr) {
		                console.log(res);
		                if(res['error']){
		                    console.log('TODO handle error')
		                }
		            }
		        });
		}


	</script>
	<!-- get range slider css theme @ http://danielstern.ca/range.css/#/ -->
	<style scoped>
		#percentage-selector{
			width:80%;
			margin: 0px 5px;
		}
		#percentage-box{
			width: 10% ;
			margin: 0px 5px 0px 15px;
		}

		input[type=range] {
		  -webkit-appearance: none;
		  width: 100%;
		  margin: 13.8px 0;
		}
		input[type=range]:focus {
		  outline: none;
		}
		input[type=range]::-webkit-slider-runnable-track {
		  width: 100%;
		  height: 8.4px;
		  cursor: pointer;
		  box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
		  background: #3071a9;
		  border-radius: 1.3px;
		  border: 0.2px solid #010101;
		}
		input[type=range]::-webkit-slider-thumb {
		  box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
		  border: 1px solid #000000;
		  height: 36px;
		  width: 16px;
		  border-radius: 3px;
		  background: #ffffff;
		  cursor: pointer;
		  -webkit-appearance: none;
		  margin-top: -14px;
		}
		input[type=range]:focus::-webkit-slider-runnable-track {
		  background: #367ebd;
		}
		input[type=range]::-moz-range-track {
		  width: 100%;
		  height: 8.4px;
		  cursor: pointer;
		  box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
		  background: #3071a9;
		  border-radius: 1.3px;
		  border: 0.2px solid #010101;
		}
		input[type=range]::-moz-range-thumb {
		  box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
		  border: 1px solid #000000;
		  height: 36px;
		  width: 16px;
		  border-radius: 3px;
		  background: #ffffff;
		  cursor: pointer;
		}
		input[type=range]::-ms-track {
		  width: 100%;
		  height: 8.4px;
		  cursor: pointer;
		  background: transparent;
		  border-color: transparent;
		  color: transparent;
		}
		input[type=range]::-ms-fill-lower {
		  background: #2a6495;
		  border: 0.2px solid #010101;
		  border-radius: 2.6px;
		  box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
		}
		input[type=range]::-ms-fill-upper {
		  background: #3071a9;
		  border: 0.2px solid #010101;
		  border-radius: 2.6px;
		  box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
		}
		input[type=range]::-ms-thumb {
		  box-shadow: 1px 1px 1px #000000, 0px 0px 1px #0d0d0d;
		  border: 1px solid #000000;
		  height: 36px;
		  width: 16px;
		  border-radius: 3px;
		  background: #ffffff;
		  cursor: pointer;
		  height: 8.4px;
		}
		input[type=range]:focus::-ms-fill-lower {
		  background: #3071a9;
		}
		input[type=range]:focus::-ms-fill-upper {
		  background: #367ebd;
		}

	</style>
</statuspanel>
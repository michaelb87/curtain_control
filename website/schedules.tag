<schedules class="app-page">
    <header class="header-bar">
        <div class="left">
            <button class="btn pull-left icon icon-arrow-back" data-navigation="$previous-page"></button>
            <h1 class="title">Zeitplan</h1>
        </div>
    </header>

    <div class="content">
        <div class="list">
        	<li class="divider">Übersicht</li>
        	<li each={schedules}>
            	<a href="#action" class="pull-right icon icon-edit" onclick={edit}></a>
            	<span class="padded-list">{n(h)}:{n(m)}:{n(s)} {action==1?'close':'open'}</span>
        	</li>
        </div>
    </div>

    <div id="editSchedulePanel" class="panel-full">
	        <header class="header-bar">
	            <a class="btn icon icon-close pull-right" href="#" data-panel-close="true"></a>
	            <h1 class="title">Panel</h1>
	        </header>

	        <div class="content">
	            <p class="padded-full">The contents of my panel go here.</p>
	        </div>
	</div>

    <script>
    	var self=this;
    	self.schedules={}; // {"error": false, "schedules": [{"dayOfWeek": 99, "h": 19, "m": 32, "s": 0, "action": 1, "percentage": 100, "active": 1, "slot": 0},{"dayOfWeek": 99, "h": 21, "m": 7, "s": 0, "action": 2, "percentage": 100, "active": 1, "slot": 1}]}

    	self.edit = function(e){
    		phonon.panel('#editSchedulePanel').open();
    	}


    	getSchedules(){
    		var req = phonon.ajax({
	            method: 'GET',
	            url: 'http://10.0.0.162/list_schedules',
	            dataType: 'json',
	            success: function(res, xhr) {
	                console.log(res, xhr);
	                self.schedules = res.schedules;
	                self.update();
	            }
	        });
    	}
    	self.getSchedules();
    	self.n = function(n){
			return n > 9 ? "" + n: "0" + n;
		}
    </script>
</schedules>
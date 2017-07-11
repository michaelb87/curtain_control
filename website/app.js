phonon.options({
    navigator: {
        defaultPage: 'home',
        animatePages: true,
        enableBrowserBackButton: true,
        templateRootDirectory: './'
    },
    i18n: null // for this example, we do not use internationalization
});


var app = phonon.navigator();

function StateHolder(){
    var self=this;
    self.defaultInterval = 10000;
    self.actionInterval = 1000;

    riot.observable(self);
    self.updateInterval = self.defaultInterval;

    self.updateState = function(){
        var req = phonon.ajax({
            method: 'GET',
            url: 'http://10.0.0.162/action?cmd=status',
            dataType: 'json',
            success: function(res, xhr) {
                if(!res['error']){
                    // curpos = min_pos + (max_pos-min_pos)/100*prc
                    // prc = (curpos-min_pois)*100/(max_pos-min_pos)
                    if(res['cur_position'] === res['target_pos']){
                        self.updateInterval = self.defaultInterval;
                        //clearInterval(self.callManager);
                        //self.callManager = setInterval(self.updateState, self.updateInterval);
                    } else{
                        self.updateInterval = self.actionInterval;
                        
                    }
                    self.callManager = setTimeout(self.updateState, self.updateInterval);//self.setDeceleratingTimeout(self.updateState, self.updateInterval,1);
                    //console.log("update interval:", self.updateInterval)
                    self.trigger('update', res);
                }
            }
        });

    }
    self.updateState(); // initial trigger
    //self.callManager = self.setDeceleratingTimeout(self.updateState, self.updateInterval,1 );
    self.on('set_action_interval', function(){
        self.updateInterval=self.actionInterval;
        clearInterval(self.callManager);
        self.callManager = setTimeout(self.updateState, self.updateInterval);

    })
}

var state = new StateHolder();

app.on({page: 'home', preventClose: false, content: null}, function(activity){
    

    var getAction = function(target, lvl){
        if(lvl==0 || target === null)
            return null;
        action = target.getAttribute('data-action');
        if(action==null)
            return getAction(target.parentElement, lvl--)
        return action;
    }

    var performAction = function(action, data){
        state.trigger('set_action_interval');
        var req = phonon.ajax({
            method: 'GET',
            url: 'http://10.0.0.162/action?cmd='+action,
            dataType: 'json',
            success: function(res, xhr) {
                if(res['error']){
                    console.log('TODO handle error')
                }
            }
        });

    }

    var onAction = function(evt) {
        var target = evt.target;
        var action = getAction(target, 3)
        switch(action){
            case 'open': performAction('open', null); break;
            case 'close': performAction('close', null); break;
            case 'stop': performAction('stop', null); break;
            case 'calibrate': performAction('calibrate', null); break;
        }
        evt.preventDefault();
    }

    activity.onCreate(function() {
        document.querySelectorAll('.action').on('tap', onAction);
        document.querySelectorAll('.action').on('click', onAction);
    });
});


app.on({page: 'schedules', preventClose: false, content: null, readyDelay: 1});


app.start();
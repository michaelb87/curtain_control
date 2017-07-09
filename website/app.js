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
        var req = phonon.ajax({
            method: 'GET',
            url: 'http://10.0.0.162/action?cmd='+action,
            dataType: 'json',
            success: function(res, xhr) {
                console.log(res);
            }/*,
            error: function(res, flagError, xhr) {
                console.error(flagError);
                console.log(res);
            }*/
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
app.start();
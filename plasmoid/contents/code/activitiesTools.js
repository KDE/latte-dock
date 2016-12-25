/*
 *  Copyright 2013 Michail Vourlakos <mvourlakos@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

var currentActivity;
var launchersOnActivities;
//it is used as the first item in the stringList in order to check the list is ok
var indicator = 'multi';
var plasmoid;

function restoreLaunchers(){
    ///this is a stringlist of types activityId, number of launchers, launchers
    if(plasmoid && plasmoid.configuration && currentActivity != "00000000-0000-0000-0000-000000000000"){
        var values = plasmoid.configuration.launchers;
        values = values.split(";");
        var returnedStringList = [];

    //    console.log("----------------------- Restoring ---------------------");
     //   console.log("Full Restoration:"+values);
        var type = values.splice(0,1);

        //  console.log("Pass 1 - " +type);
        if (type == indicator){
            //  console.log("Pass 2");
            while (values.length > 2){
                //   console.log("Pass 3 - "+values);
                var actId = values[0];
                var subLaunchers = values.splice(2,values[1]);
            //  console.log("To Be Restored launchers, "+actId+ ", "+subLaunchers.length+", "+subLaunchers);

                var activityRecord = get(actId);
                if(activityRecord){
                    //      console.log("Activity item found...");
                    if(activityRecord.launchers)
                        activityRecord.launchers.splice(0,activityRecord.launchers.length);

                    activityRecord.launchers = subLaunchers;
                }
                else{
                    //      console.log("Activity item is added...");
                    var res = {id:values[0], launchers:subLaunchers};
                    launchersOnActivities.push(res);
                }

                values.splice(0,2);

            }
        }

        if (get('*'))
            returnedStringList = returnedStringList.concat(get('*').launchers);
        else{
            var result = {id:'*', launchers:[]};
            launchersOnActivities.push(result);
        }

        if (get(currentActivity))
            returnedStringList = returnedStringList.concat(get(currentActivity).launchers);

        //     console.log("Restored Strings:"+returnedStringList);

        return returnedStringList;
    }
    else
        return [];
}

function saveLaunchers(){
  //  console.log("----------------------- Saving ---------------------");

    var returnedStringList = [];
    returnedStringList.push(indicator);

    //  console.log("Array Size:"+launchersOnActivities.length);
    var size = launchersOnActivities.length;
    for(var i=size-1; i>=0; --i){
        var activitySaving = get(launchersOnActivities[i].id);
     //   console.log("Saving, "+activitySaving.id + " - "+activitySaving.launchers.length+" - "+activitySaving.launchers);
        if(activitySaving.launchers.length>0){
        /*    console.log("------- "+activitySaving.id+" ----------");
            for(var j=0; j<activitySaving.launchers.length; ++j){
                console.log(activitySaving.launchers[j]);
            }*/

            returnedStringList = returnedStringList.concat(String(activitySaving.id));
            returnedStringList = returnedStringList.concat(String(activitySaving.launchers.length));
            // this creates segmentation faults in some cases!!!!
          //  returnedStringList = returnedStringList.concat(activitySaving.launchers);

            //this fixes the segmentation fault of previous command....
            for(var j=0; j<activitySaving.launchers.length; ++j){
                returnedStringList.push(String(activitySaving.launchers[j]));
            }

            ///check returnedStringList integrity
     /*       for(var j=0; j<returnedStringList.length; ++j){
                console.log("--- "+j+". "+returnedStringList[j]);
            }*/
        }
    }

  /*  console.log("I am out..... "+returnedStringList.length);
    for(var i=0; i<returnedStringList.length; ++i){
        console.log(i+". "+returnedStringList[i]);
    }*/

    // console.log("IMPORTANT SAVED LIST: "+returnedStringList);

    plasmoid.configuration.launchers = returnedStringList.join(";");
}

function updateLaunchers(launcherList){
   // console.log("----------------------- Updating ---------------------");
  //  console.log("---- Full Launchers ------");
   /* for(var j=0; j<launcherList.length; ++j){
        console.log(launcherList[j]);
    } */


    var tempList;
    if(launcherList.length > 0){
        tempList = launcherList.slice(0);
    }
    else{
        tempList =[];
    }

    // console.log("IMPORTANT SENT LIST: "+tempList.length+" - "+launcherList.length+" - "+tempList);
    //
    //  console.log("In memory Defaults:" + get('*').launchers);
    //  if(get(currentActivity))
    //    console.log("In memory Current: "+get(currentActivity).launchers.length+ ' - ' + get(currentActivity).launchers);

    var currentList = get(currentActivity);

    var resultedCurrent = [];
    // console.log("-------------");

    for(var i=tempList.length-1; i>=0; --i){
        var index=-1;
        if(currentList){
            index = getIndex(String(tempList[i]), currentList.launchers);
        }

        if(index >=0 || isInSpecificActivity(tempList[i])){
            var result = tempList.splice(i,1);
            resultedCurrent.unshift(String(result));
        }
    }

   // console.log("Resulted Current: "+resultedCurrent);

 /*   console.log("---- To Be Set List ------");
    for(var j=0; j<resultedCurrent.length; ++j){
        console.log(resultedCurrent[j]);
    }*/

    setActivityLaunchers(resultedCurrent, currentActivity);
    setDefaultLaunchers(tempList);

    saveLaunchers();
}

/////////////
function cleanupRecords(activities){
    for(var i=0; i<launchersOnActivities.length; ++i){
        if(launchersOnActivities[i].id != '*'){
            var index = getIndex(launchersOnActivities[i].id, activities);
            if(index < 0){
                console.log("Orphaned Activity Settings removed:"+launchersOnActivities[i].id);
                launchersOnActivities.splice(index,1);
            }
        }
    }
    saveLaunchers();
}


function getIndex(id, list){
    if(list){
        for(var i=0; i<list.length; ++i){
            if(id == list[i])
                return i;
        }
    }

    return -1;
}

function get(actId){
    for(var i=0; i<launchersOnActivities.length; ++i){
        if(launchersOnActivities[i].id == actId)
            return launchersOnActivities[i];
    }

    return;
}

function isOnAllActivities(id){
    var defaults =  get('*').launchers;
    var index = getIndex(String(id),defaults);
    if (index>=0)
        return true;

    //  console.log("Check exists:"+id+ " - "+ "pos:"+index+" ,,, "+ get('*').activities);
    return false;
}

function isInSpecificActivity(id){
    for(var i=0; i<launchersOnActivities.length; ++i){
        var activity = launchersOnActivities[i];
        if(activity.id != "*"){
            var launchers = activity.launchers;
            var index = getIndex(String(id),launchers)
            if (index>=0)
                return true;
        }
    }

    return false;
}


function setDefaultLaunchers(launchersList){
  //  console.log("Set Default Launchers");
    if(!get('*')){
        var result = {id:'*', launchers:launchersList};
        launchersOnActivities.push(result);
    }
    else{
        if (get('*').launchers)
            get('*').launchers.splice(0,get('*').launchers.length);
        get('*').launchers=launchersList;
    }

    // console.log("Default:::: "+get('*').launchers);
}

function setActivityLaunchers(launchersList, actId){
   // console.log("Set Activity Launchers");
    var currentList = get(actId);
    //  console.log("-------------");
    //  console.log("ResultedForMemory: "+launchersList);
    if(currentList){
        if(currentList.launchers)
            currentList.launchers.splice(0,currentList.launchers.length);

      //  console.log("list exists");
        currentList.launchers=launchersList;
    }
    else{
    //    console.log("new list");
        var res = {id:actId, launchers:launchersList};
        launchersOnActivities.push(res);
    }

 /*   console.log("New Memory List: "+get(actId).launchers);
    for(var j=0; j<get(actId).launchers.length; ++j){
        console.log(get(actId).launchers[j]);
    } */
}

///from launcher to All Activities to the current only and vice versa
function toggleLauncherState(id){
    if (isOnAllActivities(id)){
        removeFromList(id,'*')
        addToList(id, currentActivity);
    }
    else{
        removeFromAllSimpleLists(id);
        addToList(id, '*');
    }

    saveLaunchers();
}

function removeLauncher(id){
    if(get(currentActivity))
        removeFromList(id, currentActivity);

    removeFromList(id, '*');
}

function removeFromAllSimpleLists(id){
    for(var i=0; i<launchersOnActivities.length; ++i){
        var activity = launchersOnActivities[i];
        if(activity.id != "*"){
            removeFromList(id, activity.id);
        }
    }
}


function removeFromList(id, activityId){
    var list = get(activityId);
    if(list){
        var activityList = list.launchers;
        var index = getIndex(String(id),activityList);

        if(index >= 0){
            activityList.splice(index, 1);
        }
    }
}

function addToList(id, activityId){
    var activity = get(activityId);
    if(!activity){
        var res = {id:activityId, launchers:[]};
        res.launchers.push(id);
        launchersOnActivities.push(res);
    }
    else{
        var launcherList = activity.launchers;
        var index = getIndex(id,launcherList);
        if(index<0){
            launcherList.push(id);
        }

    }
}

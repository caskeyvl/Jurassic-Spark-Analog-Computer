# Jurassic Spark Analog Computer
This will be the working git repository containing all documentation (tex files, powerpoints, planning documents, etc) as well as any necessary design files for the analog computer.

## Repository Usage

### First time usage/cloning
To begin working on this repository, first make sure that you have properly cloned the repository using the following command: 
```
git clone https://github.com/caskeyvl/Jurassic-Spark-Analog-Computer.git
```

### Create new branch for new work module

If you want to create a new branch for a new aspect of the project (i.e. pcb/integrator_module), run the following commands: 

```
git checkout main	
git pull origin main			
git checkout -b <branch-name> 	
```

Please only choose this option if you are 100% sure that a new branch is required, and have talked to Logan about adding this new branch.

### Working on an existing branch

If you want to continue work on an existing branch, run the following commands: 

```
git checkout <branch-name>		
git pull origin <branch-name> 		
```

### Committing and pushing changes

As you work, if you finish a significant amount of work and want git to track a new file (e.g. integrator_sim.wxsch), first add the file to your locally cloned git directory, then run the following commands:

```
git add <files> 
git commit -m "Commit message"
git push origin <branch-name>
```

So, for example, if I finished the integrator simulation using SIMetrix, and want to add that to the spice branch: 

``` 
git add integrator_sim.wxsch
git commit -m "integrator spice simulation added"
git push origin spice
```

Please, TRIPLE CHECK that you are in the correct branch before commiting and pushing your changes (i.e. DO NOT push directly to main). This will make merge conflicts much more annoying to solve, and gives us the ability to modularize and further version control our work. Merges to main will happen through pull requests ONLY, and the only person who is allowed to push to main is Logan (mainly for adding to the readme file). 

### Work being finished/pull requests

If you have finished work on a branch, and feel that branch is ready to be merged to main, follow these steps: 

1. Go to GitHub repository → "Compare & pull request"→ fill in the description
2. Assign Logan Caskey as the reviewer

At this point, I (Logan) will go through the pull requests that have been made, and make the full merge to main if branch is ready to do so.

If you are not confident about creating this pull request, please just reach out to me (Logan) and I will deal with the pull request myself and merging to make sure nothing breaks, however ideally I would like everyone comfortable with pull requests to lighten the workload on myself in merging work.  

### Important notes

- When pushing to a new branch for the first time, GitHub will ask you if you'd like to make a pull request for this push. DO NOT click yes, pull requests in our workflow are reserved for when significant chunks of work are done (i.e. spice simulation, PCB schematic finished, etc). 
- When making a pull request, if you accidentally merge, GitHub will ask you if you'd like to delete the branch. DO NOT delete the branch. I will go in and turn old, finished branches to inactive, for version control purposes. 
- Some of this is TBD, and may be subject to change as I learn more about GitHub, and how to properly version control this. If you don't know how something works, ASK ME. We will figure it out together, but please don't try and figure things out on your own at the risk of breaking the GitHub and causing headaches for us later as we try and fix the repository and move it to a previous version. 
 
## Useful Links

[Running BOM doc](https://docs.google.com/spreadsheets/d/1X1U5svekkfRvvx4M4c1IeXdciezWEVZrIU-C1A8QEh0/edit?pli=1&gid=2117376175#gid=2117376175)

[General brainstorming doc](https://docs.google.com/document/d/15wyLm1f_vfKTtkXsonMv67GspUREVvMZEw6X1BMCESQ/edit?tab=t.0)

[Google doc link for updated MRD and ERD](https://docs.google.com/document/d/1tPsG1f8iEBiSaBPeFTgdizXF1T_pHfBg9JGMEwuBkTs/edit?tab=t.0)

[Sponsor Briefing week 2](https://o365coloradoedu-my.sharepoint.com/:p:/r/personal/dagl4647_colorado_edu/_layouts/15/Doc.aspx?sourcedoc=%7BE3F5A61D-4A1F-4181-83C3-D2C9ED49820E%7D&file=Sponsor_Briefing_1.pptx&wdLOR=c2051632E-E002-4DBC-AE07-C8335AE64154&nav=eyJzSWQiOjEyMTEsImNJZCI6Mjc4NjMyNzg1NywiY29tbWVudElkIjoiNEYwRTIwMzUtMDNGMS00Njc2LUEwQjYtMzNGQkRBQjY3QkQ0In0&action=edit&mobileredirect=true)

[Notion](https://www.notion.so/Gantt-Chart-Jurassic-Spark-270cb7734eae80eb8b34f487de05fe35?source=copy_link)

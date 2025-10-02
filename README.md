# Jurassic Spark Analog Computer

This will be the working git repository containing all documentation (tex files, powerpoints, planning documents, etc) as well as any necessary design files for the analog computer.

## Repository Usage

### Create new branch for new work

If you want to create a new branch for a new aspect of the project (i.e. pcb/integrator_module), run the following commands: 
'''
git checkout main			# switch to main branch
git pull origin main			# update local main
git checkout -b <branch-name> 		# create branch <branch-name>, and switch to branch
'''

### Working on an existing branch

If you want to continue work on an existing branch, run the following commands: 

'''
git checkout <branch-name>		# switch to existing branch
git pull origin <branch-name> 		# update local with current branch state
'''

### Committing and pushing changes

As you work, if you finish a file (e.g. integrator_sim.wxsch) and want to add it to the branch, run the following commands:

'''
git add <files> 
git commit -m "Commit message"
git push origin <branch-name>
'''

So, for example, if I finished the integrator simulation using SIMetrix, and want to add that to the spice branch: 

''' 
git add integrator_sim.wxsch
git commit -m "integrator spice simulation added"
git push origin spice
'''

### Work being finished/pull requests

If you have finished work on a branch, and that branch is ready to be merged to the main branch, follow these steps:

1. Go to GitHub repository → "Compare & pull request"→ fill in the description
2. Assign Logan Caskey as the reviewer

At this point Logan will go through the pull requests that have been made, and make the full merge to main if branch is ready to do so.

### Important notes

- When pushing to a new branch for the first time, GitHub will ask you if you'd like to make a pull request for this push. DO NOT click yes, pull requests in our workflow are reserved for when significant chunks of work are done (i.e. spice simulation, PCB schematic finished, etc). 
- When making a pull request, if you accidentally merge, GitHub will ask you if you'd like to delete the branch. DO NOT delete the branch. I will go in and turn old, finished branches to inactive, for version control purposes. 
- Some of this is TBD, and may be subject to change as I learn more about GitHub, and how to properly version control this. If you don't know how something works, ASK ME. We will figure it out together, but please don't try and figure things out on your own at the risk of breaking the GitHub and causing headaches for us later as we try and fix the repository and move it to a previous version. 
 
## Useful Links

[General brainstorming doc](https://docs.google.com/document/d/15wyLm1f_vfKTtkXsonMv67GspUREVvMZEw6X1BMCESQ/edit?tab=t.0)

[Google doc link for updated MRD and ERD](https://docs.google.com/document/d/1tPsG1f8iEBiSaBPeFTgdizXF1T_pHfBg9JGMEwuBkTs/edit?tab=t.0)

[Sponsor Briefing week 2](https://o365coloradoedu-my.sharepoint.com/:p:/r/personal/dagl4647_colorado_edu/_layouts/15/Doc.aspx?sourcedoc=%7BE3F5A61D-4A1F-4181-83C3-D2C9ED49820E%7D&file=Sponsor_Briefing_1.pptx&wdLOR=c2051632E-E002-4DBC-AE07-C8335AE64154&nav=eyJzSWQiOjEyMTEsImNJZCI6Mjc4NjMyNzg1NywiY29tbWVudElkIjoiNEYwRTIwMzUtMDNGMS00Njc2LUEwQjYtMzNGQkRBQjY3QkQ0In0&action=edit&mobileredirect=true)

[Notion](https://www.notion.so/Gantt-Chart-Jurassic-Spark-270cb7734eae80eb8b34f487de05fe35?source=copy_link)
